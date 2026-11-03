// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>

namespace NMib::NSystem
{
	struct CCPUUsage
	{
		CCPUUsage();
		~CCPUUsage();

		CSystemCPUUsage f_GetUsage(bool &_bChanged);
		CSystemCPUUsage f_GetUsage();

	private:
		void *mp_pHandle;
	};

	struct CCPUUsageMonitor : protected NThread::CThread
	{
		CCPUUsageMonitor(fp32 _SampleInterval, fp32 _AverageInterval);
		~CCPUUsageMonitor();

		CSystemCPUUsage f_GetLast();

		CSystemCPUUsage f_GetAverage();
		CSystemCPUUsage f_GetMin();
		CSystemCPUUsage f_GetMax();

	private:
		CCPUUsage mp_CPUUsage;
		fp32 mp_SampleInterval;
		zmint mp_CurrentHistory;
		zmint mp_nHistory;
		NContainer::TCVector<CSystemCPUUsage> mp_History;
		NThread::CMutual mp_Lock;
		CSystemCPUUsage mp_Last;
		CSystemCPUUsage mp_Average;
		CSystemCPUUsage mp_Min;
		CSystemCPUUsage mp_Max;
		aint f_Main();
		NStr::CStr f_GetThreadName();
	};
}
