#ifndef __SUBCPU_H__
#define __SUBCPU_H__

#include <pdh.h>

class SUBCPU
{
	public :

		SUBCPU();
		SUBCPU( const SUBCPU* );
		~SUBCPU();

	public :

		bool Init();
		void Release();
		void Frame();
		int GetCPUPercent();
		bool GetCanReadCPU();

	private :

		bool m_CanReadCPU;
		HQUERY m_QueryHandle;
		HCOUNTER m_CounterHandle;
		unsigned long m_LastSampleTime;
		long m_CPUUsage;
};

#endif