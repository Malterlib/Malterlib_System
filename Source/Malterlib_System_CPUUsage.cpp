// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include "Malterlib_System_CPUUsage.h"

namespace NMib::NSystem
{
	CCPUUsage::CCPUUsage()
		: m_pHandle(nullptr)
	{
		m_pHandle = NMib::NSys::fg_System_CPUUsageMonitor_Open();
	}

	CCPUUsage::~CCPUUsage()
	{
		if (m_pHandle)
		{
			NMib::NSys::fg_System_CPUUsageMonitor_Close(m_pHandle);
			m_pHandle = nullptr;
		}
	}

	CSystemCPUUsage CCPUUsage::f_GetUsage(bool &_bChanged)
	{
		return NMib::NSys::fg_System_CPUUsageMonitor_GetUsage(m_pHandle, _bChanged);
	}

	CSystemCPUUsage CCPUUsage::f_GetUsage()
	{
		bool bChanged;
		return NMib::NSys::fg_System_CPUUsageMonitor_GetUsage(m_pHandle, bChanged);
	}

	CCPUUsageMonitor::CCPUUsageMonitor(fp32 _SampleInterval, fp32 _AverageInterval)
		: m_SampleInterval(_SampleInterval)
	{
		DMibRequire(m_SampleInterval > 0.0);
		DMibRequire(_AverageInterval >= m_SampleInterval);

		umint nHistory = fg_Max((_AverageInterval / _SampleInterval).f_ToIntRound(), 1);
		m_History.f_SetLen(nHistory);

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
			auto Usage = m_CPUUsage.f_GetUsage();

			umint nHistory = m_History.f_GetLen();
			m_History[m_CurrentHistory] = Usage;
			m_CurrentHistory = (m_CurrentHistory + 1) % nHistory;
			if (m_nHistory < nHistory)
				++m_nHistory;

			CSystemCPUUsage Average;
			CSystemCPUUsage Min(1.0);
			CSystemCPUUsage Max(0.0);

			for (umint i = 0; i < m_nHistory; ++i)
			{
				auto &This = m_History[i];
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

			Average.m_Idle /= fp32(m_nHistory);
			Average.m_User /= fp32(m_nHistory);
			Average.m_Kernel /= fp32(m_nHistory);

			{
				DMibLock(m_Lock);
				m_Last = Usage;
				m_Average = Average;
				m_Min = Min;
				m_Max = Max;
			}

			m_EventWantQuit.f_WaitTimeout(m_SampleInterval);
		}
		return 0;
	}

	CSystemCPUUsage CCPUUsageMonitor::f_GetLast()
	{
		DMibLock(m_Lock);
		return m_Last;
	}

	CSystemCPUUsage CCPUUsageMonitor::f_GetAverage()
	{
		DMibLock(m_Lock);
		return m_Average;
	}

	CSystemCPUUsage CCPUUsageMonitor::f_GetMin()
	{
		DMibLock(m_Lock);
		return m_Min;
	}

	CSystemCPUUsage CCPUUsageMonitor::f_GetMax()
	{
		DMibLock(m_Lock);
		return m_Max;
	}
}
