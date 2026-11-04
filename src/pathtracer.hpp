#pragma once

#include <vector>
#include <cmath>
#include <omp.h>
#include <cassert>
#include "renderer.hpp"
#include "rng.hpp"

class PathTracer : public AbstractRenderer
{
private:
	Vec3f ComputeDirectLight(const Vec3f& surfacePoint, const Vec3f& normal, const Vec3f& incomingDir, const Material& mat)
	{
		CoordinateFrame frame;
		frame.SetFromZ(normal);

		Vec3f LoDirect(0.f);

		for (int i = 0; i < mScene.GetLightCount(); i++)
		{
			const AbstractLight* light = mScene.GetLightPtr(i);

			assert(light != nullptr);

			auto [lightPoint, intensity, lightPdf] = light->SamplePointOnLight(surfacePoint, mRandomGenerator);

			if (lightPdf <= 0.f || intensity.Max() <= 0.f) {
				continue;
			}

			const Vec3f toLight = lightPoint - surfacePoint;
			const float lightDistance = toLight.Length();

			if (lightDistance <= 0.f) {
				continue;
			}

			const Vec3f worldLightDir = toLight / lightDistance;
			const Vec3f localLightDir = frame.ToLocal(worldLightDir);

			const float cosTheta = localLightDir.z;

			if (cosTheta <= 0.f) {
				continue;
			}

			Ray shadowRay(surfacePoint, worldLightDir, EPSILON_RAY);

			if (!mScene.FindAnyIntersection(shadowRay, lightDistance))
			{
				LoDirect += intensity * mat.EvaluateBRDF(incomingDir, localLightDir) * cosTheta / lightPdf;
			}
		}

		return LoDirect;
	};

public:
	Rng mRandomGenerator;

	PathTracer(
		const Scene& aScene,
		int aSeed = 1234
	) :
		AbstractRenderer(aScene), mRandomGenerator(aSeed)
	{
	}

	virtual void RunIteration(int iteration)
	{
		const int resolutionX = int(mScene.mCamera.mResolution.x);
		const int resolutionY = int(mScene.mCamera.mResolution.y);

		for (int pixelID = 0; pixelID < resolutionX * resolutionY; pixelID++)
		{
			const int x = pixelID % resolutionX;
			const int y = pixelID / resolutionX;

			const Vec2f sample = Vec2f(float(x), float(y)) + mRandomGenerator.GetVec2f();

			Ray ray = mScene.mCamera.GenerateRay(sample);

			auto intersection = mScene.FindClosestIntersection(ray);

			if (!intersection) {
				if (const BackgroundLight* background = mScene.GetBackground()) {
					mFramebuffer.AddColor(sample, background->Evaluate(ray.direction));
				}
				continue;
			}

			if (intersection->lightID >= 0) {
				const AbstractLight* light = mScene.GetLightPtr(intersection->lightID);
				mFramebuffer.AddColor(sample, light->Evaluate(ray.direction));
				continue;
			}

			const Vec3f surfacePoint = ray.origin + ray.direction * intersection->distance;

			const Material& mat = mScene.GetMaterial(intersection->materialID);

			CoordinateFrame frame;
			frame.SetFromZ(intersection->normal);

			const Vec3f incomingDir = frame.ToLocal(-ray.direction);

			const Vec3f LoDirect = ComputeDirectLight(surfacePoint, intersection->normal, incomingDir, mat);

			mFramebuffer.AddColor(sample, LoDirect);
		}

		mIterations++;
	}
};
