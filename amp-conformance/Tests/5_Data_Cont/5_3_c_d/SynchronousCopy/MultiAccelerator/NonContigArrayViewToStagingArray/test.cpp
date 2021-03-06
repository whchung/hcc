// Copyright (c) Microsoft
// All rights reserved
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache Version 2.0 License for specific language governing permissions and limitations under the License.

/// <summary>Copy from non contiguous Array View to staging Array</summary>

#include "./../../../CopyTestFlow.h"
#include <amptest_main.h>

using namespace Concurrency;
using namespace Concurrency::Test;

void print_access_type_tuple(std::tuple<access_type>& tup)
{
	Log(LogType::Info, true) << "CPU Access Types: (" << std::get<0>(tup) << ")" << std::endl;
}

class NonContigArrayViewToStagingArrayTests
{
private:
	accelerator cpu_acc;
	accelerator gpu_acc1;
	accelerator gpu_acc2;

	std::vector<std::tuple<access_type>> access_types_vec;

public:
	NonContigArrayViewToStagingArrayTests()
	{
		cpu_acc = accelerator(accelerator::cpu_accelerator);
		gpu_acc1 = require_device_for<DATA_TYPE>(device_flags::NOT_SPECIFIED, false);
		gpu_acc2 = require_device_for<DATA_TYPE>(gpu_acc1, device_flags::NOT_SPECIFIED, false);

		if(gpu_acc1.get_supports_cpu_shared_memory())
		{
			WLog(LogType::Info, true) << "Accelerator " << gpu_acc1.get_description() << " supports zero copy" << std::endl;

			// Set the default cpu access type for this accelerator
			gpu_acc1.set_default_cpu_access_type(DEF_ACCESS_TYPE1);

			access_types_vec.push_back(std::make_tuple(access_type_none));
			access_types_vec.push_back(std::make_tuple(access_type_read));
			access_types_vec.push_back(std::make_tuple(access_type_write));
			access_types_vec.push_back(std::make_tuple(access_type_read_write));
		}
		else
		{
			access_types_vec.push_back(std::make_tuple(access_type_auto));
		}

		if(gpu_acc2.get_supports_cpu_shared_memory())
		{
			WLog(LogType::Info, true) << "Accelerator " << gpu_acc2.get_description() << " supports zero copy" << std::endl;

			// Set the default cpu access type for this accelerator
			gpu_acc2.set_default_cpu_access_type(DEF_ACCESS_TYPE2);
		}
	}

	runall_result Gpu1AccViewToGpu2AccView()
	{
		accelerator_view cpu_av = cpu_acc.get_default_view();
		accelerator_view arr_av = gpu_acc1.get_default_view();
		accelerator_view stg_arr_av = gpu_acc2.get_default_view();

		runall_result res;

		for(auto a_t_tuple : access_types_vec)
		{
			print_access_type_tuple(a_t_tuple);
			res &= CopyAndVerifyFromNonContigArrayViewToStagingArray<DATA_TYPE, RANK>(cpu_av, arr_av, stg_arr_av, std::get<0>(a_t_tuple));
		}

		return res;
	}
};

runall_result test_main()
{
	NonContigArrayViewToStagingArrayTests tests;
	runall_result res;

	res &= REPORT_RESULT(tests.Gpu1AccViewToGpu2AccView());

	return res;
}

