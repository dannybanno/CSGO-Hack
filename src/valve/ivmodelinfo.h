#pragma once
#include "../util/memory.h"

#include "cvector.h"


// thx qo0!
enum EHitboxIndex : int
{
	HITBOX_INVALID = -1,
	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_THORAX,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	HITBOX_MAX,
};

class CStudioBone
{
public:

};

class CStudioBoundingBox
{
public:
	int bone;
	int group;
	CVector bbMin;
	CVector bbMax;
	int hitboxNameIndex;
	CVector offsetRotation;
	float capsuleRadius;
	int unused[4];
};

class CStudioHitboxSet
{
public:
	int nameIndex;
	int numHitboxes;
	int hitboxIndex;


	inline const char* GetName() const noexcept 
	{
		return reinterpret_cast<char*>(std::uintptr_t(this) + nameIndex);
	}

	inline CStudioBoundingBox* GetHitbox(int index) noexcept
	{
		if (index < 0 || index >= numHitboxes)
			return nullptr;

		return reinterpret_cast<CStudioBoundingBox*>(reinterpret_cast<std::uint8_t*>(this) + hitboxIndex) + index;
	}
};

class Quaternion;

class RadianEuler
{
public:
	inline RadianEuler(void) { }

	inline RadianEuler(float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;
	}

	inline RadianEuler(Quaternion const& q);    // evil auto type promotion!!!

	// Initialization
	inline void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
	{
		x = ix;
		y = iy;
		z = iz;
	}


	bool IsValid() const;

	void Invalidate();

	inline float* Base()
	{
		return &x;
	}

	inline const float* Base() const
	{
		return &x;
	}

	// array access...
	float operator[](int i) const;

	float& operator[](int i);

	float x, y, z;
};

class Quaternion				// same data-layout as engine's vec4_t,
{                                //		which is a float[4]
public:
	inline Quaternion(void) { }

	inline Quaternion(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) { }

	inline Quaternion(RadianEuler const& angle);    // evil auto type promotion!!!

	inline void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f, float iw = 0.0f)
	{
		x = ix;
		y = iy;
		z = iz;
		w = iw;
	}

	bool IsValid() const;

	void Invalidate();

	bool operator==(const Quaternion& src) const;

	bool operator!=(const Quaternion& src) const;

	float* Base()
	{
		return (float*)this;
	}

	const float* Base() const
	{
		return (float*)this;
	}

	// array access...
	float operator[](int i) const;

	float& operator[](int i);

	float x, y, z, w;
};




struct mstudiobone_t {
	int sznameindex;

	inline char* const pszName(void) const
	{
		return ((char*)this) + sznameindex;
	}

	int parent;        // parent bone
	int bonecontroller[6];    // bone controller index, -1 == none

	// default values
	CVector pos;
	Quaternion quat;
	RadianEuler rot;
	// compression scale
	CVector posscale;
	CVector rotscale;

	CMatrix3x4 poseToBone;
	Quaternion qAlignment;
	int flags;
	int proctype;
	int procindex;        // procedural rule
	mutable int physicsbone;    // index into physically simulated bone

	inline void* pProcedure() const
	{
		if (procindex == 0)
			return NULL;
		else
			return (void*)(((unsigned char*)this) + procindex);
	};

	int surfacepropidx;    // index into string tablefor property name

	inline char* const pszSurfaceProp(void) const
	{
		return ((char*)this) + surfacepropidx;
	}

	inline int GetSurfaceProp(void) const
	{
		return surfacepropLookup;
	}

	int contents;        // See BSPFlags.h for the contents flags
	int surfacepropLookup;    // this index must be cached by the loader, not saved in the file
	int unused[7];        // remove as appropriate
};

#define Assert( _exp ) ((void)0)

class CStudioHdr
{
public:
	inline CStudioBone* GetBone(int index) noexcept
	{
		if (index < 0 || index >= numBones)
			return nullptr;

		return reinterpret_cast<CStudioBone*>(std::uintptr_t(this) + boneIndex) + index;
	}

	inline CStudioHitboxSet* GetHitboxSet(int index) noexcept
	{
		if (index < 0 || index >= numHitboxSets)
			return nullptr;

		return reinterpret_cast<CStudioHitboxSet*>(std::uintptr_t(this) + hitboxSetIndex) + index;
	}

	int id;
	int version;
	int checksum;
	char name[64];
	int length;

	CVector eyePosition;
	CVector illumPosition;
	CVector hullMin;
	CVector hullMax;
	CVector bbMin;
	CVector bbMax;

	int flags;
	int numBones;
	int boneIndex;
	int numBoneControllers;
	int boneControllerIndex;
	int numHitboxSets;
	int hitboxSetIndex;

	inline mstudiobone_t* pBone(int i) const
	{
		Assert(i >= 0 && i < numbones);
		return (mstudiobone_t*)(((unsigned char*)this) + boneIndex) + i;
	};

};

class CModel
{
public:
	std::int32_t handle;
	char name[260];
	std::int32_t loadFlags;
	std::int32_t serverCount;
	std::int32_t type;
	std::int32_t flags;
	CVector mins;
	CVector maxs;
	float radius;
};

class IVModelInfo
{
public:
	enum ERenderFlags : std::uint32_t
	{
		RENDER_FLAGS_DISABLE_RENDERING = 0x01,
		RENDER_FLAGS_HASCHANGED = 0x02,
		RENDER_FLAGS_ALTERNATE_SORTING = 0x04,
		RENDER_FLAGS_RENDER_WITH_VIEWMODELS = 0x08,
		RENDER_FLAGS_BLOAT_BOUNDS = 0x10,
		RENDER_FLAGS_BOUNDS_VALID = 0x20,
		RENDER_FLAGS_BOUNDS_ALWAYS_RECOMPUTE = 0x40,
		RENDER_FLAGS_IS_SPRITE = 0x80,
		RENDER_FLAGS_FORCE_OPAQUE_PASS = 0x100,
	};

	constexpr const char* GetModelName(const model_t* model) noexcept
	{
		return memory::Call<const char*>(this, 3, model);
	}

	constexpr CStudioHdr* GetStudioModel(const CModel* model) noexcept
	{
		return memory::Call<CStudioHdr*>(this, 32, model);
	}
};
