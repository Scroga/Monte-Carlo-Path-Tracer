#pragma once 

#include <utility>
#include <tuple>
#include <stdexcept>
#include "math.hpp"
#include "rng.hpp"

class Material
{
private:
	Vec3f sampleDiffuse(Rng& rng) const {
		const float r1 = rng.GetUniformRand(0.f, 1.f);
		const float r2 = rng.GetUniformRand(0.f, 1.f);

		const float sinTheta = sqrt(1.f - r2);
		const float cosTheta = sqrt(r2);
		const float phi = 2. * PI_F * r1;

		Vec3f dir{
			cos(phi) * sinTheta,
			sin(phi) * sinTheta,
			cosTheta
		};

		return dir;
	}

	Vec3f sampleSpecular(const Vec3f& reflectedDir, Rng& rng) const {
		const float r1 = rng.GetUniformRand(0.f, 1.f);
		const float r2 = rng.GetUniformRand(0.f, 1.f);

		const float cosTheta = pow(r2, 1.f / (mPhongExponent + 1.f));
		const float sinTheta = sqrt(1.f - cosTheta * cosTheta);

		const float phi = 2. * PI_F * r1;

		Vec3f dir{
			cos(phi) * sinTheta,
			sin(phi) * sinTheta,
			cosTheta
		};

		CoordinateFrame lobeFrame;
		lobeFrame.SetFromZ(reflectedDir);

		return lobeFrame.ToWorld(dir);
	}

public:
	Vec3f mDiffuseReflectance;
	Vec3f mPhongReflectance;
	float mPhongExponent;

	Material()
	{
		Reset();
	}

	void Reset()
	{
		mDiffuseReflectance = Vec3f(0);
		mPhongReflectance = Vec3f(0);
		mPhongExponent = 1.f;
	}

	/**
	 * Randomly chooses an outgoing direction that is reflected from the material surface
	 * Arguments:
	 *  - incomingDir = a normalized direction towards the previous (origin) point in the scene
	 *  - rng = random generator
	 * Returns:
	 *  - a randomly sampled reflected outgoing direction
	 *  - the intensity corresponding to the reflected light
	 *  - the probability density (PDF) of choosing this direction
	 */
	std::tuple<Vec3f, Vec3f, float> SampleReflectedDirection(const Vec3f& incomingDir, Rng& rng) const {
		if (incomingDir.z <= 0.f) {
			return { Vec3f(0.f), Vec3f(0.f), 0.f };
		}

		float probDiffuse = mDiffuseReflectance.Max();
		float probSpecular = mPhongReflectance.Max();
		float total = probDiffuse + probSpecular;

		if (total <= 0.f) {
			return { Vec3f{0.f}, Vec3f{0.f}, 0.f };
		}

		probDiffuse /= total;
		probSpecular /= total;

		Vec3f generatedDir{};
		const Vec3f reflectedDir{
			-incomingDir.x,
			-incomingDir.y,
			incomingDir.z
		};


		if (rng.GetUniformRand(0.f, 1.f) <= probDiffuse) {
			generatedDir = sampleDiffuse(rng);
		}
		else {
			generatedDir = sampleSpecular(reflectedDir, rng);
		}

		if (generatedDir.z <= 0.f) {
			return { generatedDir, Vec3f(0.f), 0.f };
		}

		Vec3f brdf = EvaluateBRDF(incomingDir, generatedDir);
		float pdf = PDF(incomingDir, generatedDir);

		return { generatedDir, brdf, pdf };
	}

	/**
	 * Returns the probability density corresponding to sampleReflectedDirection,
	 * i.e., what is the probability that calling sampleReflectedDirection would randomly choose the given outgoingDirection
	 * Arguments:
	 *  - incomingDir = a normalized direction towards the previous (origin) point in the scene
	 *  - outgoingDir = the randomly sampled (normalized) outgoing direction
	 */
	float PDF(const Vec3f& incomingDir, const Vec3f& outgoingDir) const {
		if (incomingDir.z <= 0.f || outgoingDir.z <= 0.f) {
			return 0.f;
		}

		float probDiffuse = mDiffuseReflectance.Max();
		float probSpecular = mPhongReflectance.Max();
		float total = probDiffuse + probSpecular;

		if (total <= 0.f) {
			return 0.f;
		}

		probDiffuse /= total;
		probSpecular /= total;

		const Vec3f reflectedDir(
			-incomingDir.x,
			-incomingDir.y,
			incomingDir.z
		);

		const float cosThetaRefl = std::max(0.f, Dot(reflectedDir, outgoingDir));

		const float pdfDiffuse = outgoingDir.z / PI_F;
		const float pdfSpecular = (mPhongExponent + 1.f) / (2.f * PI_F) * pow(cosThetaRefl, mPhongExponent);

		return probDiffuse * pdfDiffuse + probSpecular * pdfSpecular;
	}

	/**
	 * Returns the intensity corresponding to the reflected light according to this material's BRDF
	 * Arguments:
	 *  - incomingDir = a normalized direction towards the previous (origin) point in the scene
	 *  - outgoingDir = a normalized outgoing reflected direction
	 */
	Vec3f EvaluateBRDF(const Vec3f& incomingDir, const Vec3f& outgoingDir) const {
		if (incomingDir.z <= 0.f || outgoingDir.z <= 0.f) {
			return Vec3f(0.f);
		}

		const Vec3f reflectedDir(
			-incomingDir.x,
			-incomingDir.y,
			incomingDir.z
		);

		const float cosThetaRefl = std::max(0.f, Dot(reflectedDir, outgoingDir));

		const Vec3f diffuse = mDiffuseReflectance / PI_F;
		const Vec3f specular = mPhongReflectance * (mPhongExponent + 2.f) / (2.f * PI_F) * pow(cosThetaRefl, mPhongExponent);

		return diffuse + specular;
	}
};
