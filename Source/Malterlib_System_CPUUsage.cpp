// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Mib/Core/Core>
#include "Malterlib_System_CPUUsage.h"

namespace NMib::NSystem
{
	CCPUUsage::CCPUUsage()
		: mp_pHandle(nullptr)
	{
		mp_pHandle = NMib::NSys::fg_System_CPUUsageMonitor_Open();
	}

	CCPUUsage::~CCPUUsage()
	{
		if (mp_pHandle)
		{
			NMib::NSys::fg_System_CPUUsageMonitor_Close(mp_pHandle);
			mp_pHandle = nullptr;
		}
	}

	CSystemCPUUsage CCPUUsage::f_GetUsage(bool &_bChanged)
	{
		return NMib::NSys::fg_System_CPUUsageMonitor_GetUsage(mp_pHandle, _bChanged);
	}

	CSystemCPUUsage CCPUUsage::f_GetUsage()
	{
		bool bChanged;
		return NMib::NSys::fg_System_CPUUsageMonitor_GetUsage(mp_pHandle, bChanged);
	}

	CCPUUsageMonitor::CCPUUsageMonitor(fp32 _SampleInterval, fp32 _AverageInterval)
		: mp_SampleInterval(_SampleInterval)
	{
		DMibRequire(mp_SampleInterval > 0.0);
		DMibRequire(_AverageInterval >= mp_SampleInterval);

		umint nHistory = fg_Max((_AverageInterval / _SampleInterval).f_ToIntRound(), 1);
		mp_History.f_SetLen(nHistory);

		f_Start();
	}

	CCPUUsageMonitor::~CCPUUsageMonitor()
	{
		f_Stop();
	}

	NStr::CStr CCPUUsageMonitor::f_GetThreadName()
	{
		return "CCPUUsageMonitor";
	}

	aint CCPUUsageMonitor::f_Main()
	{
		while (f_GetState() != NThread::EThreadState_EventWantQuit)
		{
			auto Usage = mp_CPUUsage.f_GetUsage();

			umint nHistory = mp_History.f_GetLen();
			mp_History[mp_CurrentHistory] = Usage;
			mp_CurrentHistory = (mp_CurrentHistory + 1) % nHistory;
			if (mp_nHistory < nHistory)
				++mp_nHistory;

			CSystemCPUUsage Average;
			CSystemCPUUsage Min(1.0);
			CSystemCPUUsage Max(0.0);

			for (umint iHistory = 0; iHistory < mp_nHistory; ++iHistory)
			{
				auto &This = mp_History[iHistory];
				Min.m_Idle = fg_Min(Min.m_Idle, This.m_Idle);
				Min.m_User = fg_Min(Min.m_User, This.m_User);
				Min.m_Kernel = fg_Min(Min.m_Kernel, This.m_Kernel);

				Max.m_Idle = fg_Max(Max.m_Idle, This.m_Idle);
				Max.m_User = fg_Max(Max.m_User, This.m_User);
				Max.m_Kernel = fg_Max(Max.m_Kernel, This.m_Kernel);

				Average.m_Idle += This.m_Idle;
				Average.m_User += This.m_User;
				Average.m_Kernel += This.m_Kernel;
			}

			Average.m_Idle /= fp32(mp_nHistory);
			Average.m_User /= fp32(mp_nHistory);
			Average.m_Kernel /= fp32(mp_nHistory);

			{
				DMibLock(mp_Lock);
				mp_Last = Usage;
				mp_Average = Average;
				mp_Min = Min;
				mp_Max = Max;
			}

			m_EventWantQuit.f_WaitTimeout(mp_SampleInterval);
		}
		return 0;
	}

	CSystemCPUUsage CCPUUsageMonitor::f_GetLast()
	{
		DMibLock(mp_Lock);
		return mp_Last;
	}

	CSystemCPUUsage CCPUUsageMonitor::f_GetAverage()
	{
		DMibLock(mp_Lock);
		return mp_Average;
	}

	CSystemCPUUsage CCPUUsageMonitor::f_GetMin()
	{
		DMibLock(mp_Lock);
		return mp_Min;
	}

	CSystemCPUUsage CCPUUsageMonitor::f_GetMax()
	{
		DMibLock(mp_Lock);
		return mp_Max;
	}
}
