#pragma once

#include <vector>
#include <cmath>
#include <omp.h>
#include <cassert>
#include "renderer.hpp"
#include "rng.hpp"

class PathTracer : public AbstractRenderer
{
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

												auto [localDir, brdf, pdf] = mat.SampleReflectedDirection(incomingDir, mRandomGenerator);

												if (pdf <= 0.f || localDir.z <= 0.f) {
																continue;
												}

												const Vec3f worldDir = frame.ToWorld(localDir);

												Ray reflectedRay(surfacePoint, worldDir, EPSILON_RAY);

												auto reflectedIntersection = mScene.FindClosestIntersection(reflectedRay);

												Vec3f Li(0.f);

												if (reflectedIntersection) {
																if (reflectedIntersection->lightID >= 0) {
																				const AbstractLight* light = mScene.GetLightPtr(reflectedIntersection->lightID);
																				Li = light->Evaluate(worldDir);
																}
												}
												else {
																if (const BackgroundLight* background = mScene.GetBackground()) {
																				Li = background->Evaluate(worldDir);
																}
												}

												const float cosTheta = localDir.z;

												const Vec3f Lo = Li * brdf * cosTheta / pdf;

												mFramebuffer.AddColor(sample, Lo);
								}

								mIterations++;
				}
};
