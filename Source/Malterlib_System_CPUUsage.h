// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NSystem
{
	class CCPUUsage
	{

		void *m_pHandle;
	public:
		CCPUUsage();
		~CCPUUsage();

		CSystemCPUUsage f_GetUsage(bool &_bChanged);
		CSystemCPUUsage f_GetUsage();
	};

	class CCPUUsageMonitor : protected NThread::CThread
	{
		CCPUUsage m_CPUUsage;
		fp32 m_SampleInterval;
		zmint m_CurrentHistory;
		zmint m_nHistory;
		NContainer::TCVector<CSystemCPUUsage> m_History;
		NThread::CMutual m_Lock;
		CSystemCPUUsage m_Last;
		CSystemCPUUsage m_Average;
		CSystemCPUUsage m_Min;
		CSystemCPUUsage m_Max;
		aint f_Main();
		NStr::CStr f_GetThreadName();
	public:
		CCPUUsageMonitor(fp32 _SampleInterval, fp32 _AverageInterval);
		~CCPUUsageMonitor();

		CSystemCPUUsage f_GetLast();

		CSystemCPUUsage f_GetAverage();
		CSystemCPUUsage f_GetMin();
		CSystemCPUUsage f_GetMax();
	};
}
