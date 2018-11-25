#include <stdio.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>


extern CU_TestInfo barker_tests[];
extern CU_TestInfo general_tests[];
extern CU_TestInfo size_tests[];
extern CU_TestInfo frame_tests[];

static CU_SuiteInfo suites[] = {
	{"General", NULL, NULL, NULL, NULL, general_tests},
	{"Barker", NULL, NULL, NULL, NULL, barker_tests},
	{"Size", NULL, NULL, NULL, NULL, size_tests},
	{"Frame", NULL, NULL, NULL, NULL, frame_tests},
	CU_SUITE_INFO_NULL,
};

int main()
{
	unsigned int failed = 0;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		goto out;

	if (CUE_SUCCESS != CU_register_suites(suites))
		goto err_cleanup;

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	failed = CU_get_number_of_tests_failed();

err_cleanup:
	CU_cleanup_registry();
out:
	return CU_get_error() || failed;
}