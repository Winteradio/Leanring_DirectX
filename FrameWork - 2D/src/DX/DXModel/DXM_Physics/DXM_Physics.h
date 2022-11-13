#ifndef __DXM_PHYSICS_H__
#define __DXM_PHYSICS_H__

#include "DXM_Common.h"
#include <cmath>

struct FENCE
{
	float Range;
	float Outer;
	XMFLOAT3 InNOR;
	XMFLOAT3 OutNOR;

	MODELINFO MODEL;
};

class DXM_PHYSICS
{
	public :

		DXM_PHYSICS();
		DXM_PHYSICS( const DXM_PHYSICS* );
		~DXM_PHYSICS();

	public :

		bool Init( int, int );
		bool Frame( MODELINFO*&, int, float );
		void Release();

	private :

		void CalEulerMethod( MODELINFO*&, int, float, bool );

		void CalCollision( MODELINFO*&, int, float, bool );
		void CalCollisionFence( MODELINFO&, float );
		void CalCollisionModel( MODELINFO*&, MODELINFO&, int, int, float );

		void CalAccelerate( MODELINFO*&, int, float );
		void CalAngAccelerate( MODELINFO*&, int, float );

		void CalVelocity( MODELINFO*&, int, float );
		void CalAngVelocity( MODELINFO*&, int, float );

		void CalPosition( MODELINFO*&, int, float );
		void CalAngle( MODELINFO*&, int, float );

		void InitForce( MODELINFO*&, int );
		void SetGravityForce( XMFLOAT3&, float );
		void SetDragForce( XMFLOAT3&, XMFLOAT3, float );
		void SetCollisionVelocity( MODELINFO&, MODELINFO&, float );

		float m_GravityConstant;
		float m_SpringConstant;
		float m_DemperConstant;
		float m_DragConstant;
		float m_CollisionConstant;
		float m_ERROR;

		int m_NumFence;
		FENCE* m_Fence;
};

#endif