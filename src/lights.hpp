#pragma once

#include <vector>
#include <cmath>
#include <utility>
#include <tuple>
#include <stdexcept>
#include "math.hpp"
#include "rng.hpp"

class AbstractLight
{
public:

	/**
	 * Randomly chooses a point on the light source
	 * Arguments:
	 *  - origin = our current position in the scene
	 *  - rng = random generator
	 * Returns:
	 *  - a randomly sampled point on the light source
	 *  - the illumination intensity corresponding to the sampled direction
	 *  - the probability density (PDF) of choosing this point
	 */
	virtual std::tuple<Vec3f, Vec3f, float> SamplePointOnLight(const Vec3f& origin, Rng& rng) const {
		throw std::logic_error("Not implemented");
	}

	/**
	 * Returns the probability density corresponding to samplePointOnLight,
	 * i.e., what is the probability that calling samplePointOnLight would randomly choose the given lightPoint
	 * Arguments:
	 *  - origin = our current position in the scene
	 *  - lightPoint = the randomly sampled point on the light source
	 */
	virtual float PDF(const Vec3f& origin, const Vec3f& lightPoint) const {
		throw std::logic_error("Not implemented");
	}

	/**
	 * Returns the illumination intensity in the given direction
	 * Arguments:
	 *  - direction = direction towards the light source
	 */
	virtual Vec3f Evaluate(const Vec3f& direction) const {
		throw std::logic_error("Not implemented");
	}

	virtual ~AbstractLight() = default;
};

//////////////////////////////////////////////////////////////////////////
class AreaLight : public AbstractLight
{
public:

	AreaLight(
		const Vec3f& aP0,
		const Vec3f& aP1,
		const Vec3f& aP2)
	{
		p0 = aP0;
		e1 = aP1 - aP0;
		e2 = aP2 - aP0;

		Vec3f normal = Cross(e1, e2);
		float len = normal.Length();
		mInvArea = 2.f / len;
		mFrame.SetFromZ(normal);
	}

public:
	Vec3f p0, e1, e2;
	CoordinateFrame mFrame;
	Vec3f mRadiance;
	float mInvArea;

	std::tuple<Vec3f, Vec3f, float> SamplePointOnLight(const Vec3f& origin, Rng& rng) const override {
		const float r1 = rng.GetUniformRand(0.f, 1.f);
		const float r2 = rng.GetUniformRand(0.f, 1.f);
		const float root = std::sqrt(r1);

		const float u = 1.f - root;
		const float v = r2 * root;
		const Vec3f lightPoint = p0 + u * e1 + v * e2;

		const Vec3f toLight = lightPoint - origin;
		const float distSqr = toLight.LenSqr();
		if (distSqr <= 0.f) {
			return { lightPoint, Vec3f(0.f), mInvArea };
		}

		const Vec3f wi = toLight / std::sqrt(distSqr);
		const float cosAtLight = std::max(0.f, Dot(mFrame.mZ, -wi));
		const Vec3f intensity = mRadiance * (cosAtLight / distSqr);

		return { lightPoint, intensity, mInvArea };
	}

	float PDF(const Vec3f& origin, const Vec3f& lightPoint) const override {
		return mInvArea;
	}

	Vec3f Evaluate(const Vec3f& direction) const {
		return Dot(mFrame.mZ, -direction) > 0.f ? mRadiance : Vec3f(0.f);
	}
};

//////////////////////////////////////////////////////////////////////////
class PointLight : public AbstractLight
{
public:
	Vec3f mPosition;
	Vec3f mIntensity;

	PointLight(const Vec3f& aPosition)
	{
		mPosition = aPosition;
	}

	virtual std::tuple<Vec3f, Vec3f, float> SamplePointOnLight(const Vec3f& origin, Rng& rng) const override {
		Vec3f outgoingDirection = mPosition - origin;
		float distanceSquared = outgoingDirection.LenSqr();

		return { mPosition, mIntensity / distanceSquared, 1.f };
	}

	float PDF(const Vec3f& origin, const Vec3f& lightPoint) const override {
		return (lightPoint - mPosition).LenSqr() == 0.f ? 1.f : 0.f;
	}

	Vec3f Evaluate(const Vec3f& direction) const {
		return 0.f;
	}
};

//////////////////////////////////////////////////////////////////////////
class BackgroundLight : public AbstractLight
{
public:
	Vec3f mBackgroundColor;
	float mRadius; // we model the background light as a huge sphere around the whole scene, with a given radius

	BackgroundLight()
	{
		mBackgroundColor = Vec3f(135, 206, 250) / Vec3f(255.f);
		mRadius = 100.f; // a radius big enough to cover the whole scene
	}

	std::tuple<Vec3f, Vec3f, float> SamplePointOnLight(const Vec3f& origin, Rng& rng) const override {
		if (mRadius <= 0.f || origin.LenSqr() >= mRadius * mRadius) {
			return { Vec3f(0.f), Vec3f(0.f), 0.f };
		}

		const float r1 = rng.GetUniformRand(0.f, 1.f);
		const float r2 = rng.GetUniformRand(0.f, 1.f);

		const float z = 1.f - 2.f * r1;
		const float radial = std::sqrt(std::max(0.f, 1.f - z * z));
		const float phi = 2.f * PI_F * r2;
		const Vec3f wi(
			radial * std::cos(phi),
			radial * std::sin(phi),
			z
		);

		const float b = Dot(origin, wi);
		const float disc = b * b + mRadius * mRadius - origin.LenSqr();
		const float t = -b + std::sqrt(std::max(0.f, disc));
		const Vec3f lightPoint = origin + t * wi;

		return { lightPoint, mBackgroundColor, PDF(origin, lightPoint) };
	}

	float PDF(const Vec3f& origin, const Vec3f& lightPoint) const override {
		return 1.f / (4.f * PI_F);
	}

	Vec3f Evaluate(const Vec3f& direction) const {
		return mBackgroundColor;
	}
};
