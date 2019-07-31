/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Heiko Strathmann, Soeren Sonnenburg, Giovanni De Toni, 
 *          Sergey Lisitsyn, Saurabh Mahindre, Jacob Walker, Viktor Gal, 
 *          Leon Kuchenbecker
 */

#include <shogun/base/Parameter.h>
#include <shogun/base/progress.h>
#include <shogun/evaluation/CrossValidation.h>
#include <shogun/evaluation/CrossValidationStorage.h>
#include <shogun/evaluation/Evaluation.h>
#include <shogun/evaluation/SplittingStrategy.h>
#include <shogun/lib/List.h>
#include <shogun/lib/observers/ObservedValueTemplated.h>
#include <shogun/machine/Machine.h>
#include <shogun/mathematics/Statistics.h>
#include <shogun/lib/View.h>

using namespace shogun;

CCrossValidation::CCrossValidation() : Seedable<CMachineEvaluation>()
{
	init();
}

CCrossValidation::CCrossValidation(
    CMachine* machine, CFeatures* features, CLabels* labels,
    CSplittingStrategy* splitting_strategy, CEvaluation* evaluation_criterion)
    : Seedable<CMachineEvaluation>(
          machine, features, labels, splitting_strategy, evaluation_criterion)
{
	init();
}

CCrossValidation::CCrossValidation(
    CMachine* machine, CLabels* labels, CSplittingStrategy* splitting_strategy,
    CEvaluation* evaluation_criterion)
    : Seedable<CMachineEvaluation>(
          machine, labels, splitting_strategy, evaluation_criterion)
{
	init();
}

CCrossValidation::~CCrossValidation()
{
}

void CCrossValidation::init()
{
	m_num_runs = 1;

	SG_ADD(&m_num_runs, "num_runs", "Number of repetitions");
}

CEvaluationResult* CCrossValidation::evaluate_impl() const
{
	SGVector<float64_t> results(m_num_runs);

	/* perform all the x-val runs */
	SG_DEBUG("starting %d runs of cross-validation\n", m_num_runs)
	for (auto i : SG_PROGRESS(range(m_num_runs)))
	{
		results[i] = evaluate_one_run(i);
		SG_INFO("Result of cross-validation run %d/%d is %f\n", i+1, m_num_runs, results[i])
	}

	/* construct evaluation result */
	CCrossValidationResult* result = new CCrossValidationResult();
	result->set_mean(CStatistics::mean(results));
	if (m_num_runs > 1)
		result->set_std_dev(CStatistics::std_deviation(results));
	else
		result->set_std_dev(0);

	SG_REF(result);
	return result;
}

void CCrossValidation::set_num_runs(int32_t num_runs)
{
	if (num_runs < 1)
		SG_ERROR("%d is an illegal number of repetitions\n", num_runs)

	m_num_runs = num_runs;
}

float64_t CCrossValidation::evaluate_one_run(int64_t index) const
{
	SG_DEBUG("entering %s::evaluate_one_run()\n", get_name())
	index_t num_subsets = m_splitting_strategy->get_num_subsets();

	SG_DEBUG("building index sets for %d-fold cross-validation\n", num_subsets)
	m_splitting_strategy->build_subsets();

	SGVector<float64_t> results(num_subsets);

	#pragma omp parallel for shared(results)
	for (auto i = 0; i<num_subsets; ++i)
	{
		// only need to clone hyperparameters and settings of machine
		// model parameters are inferred/learned during training
		auto machine = make_clone(m_machine,
				ParameterProperties::HYPER | ParameterProperties::SETTING);

		SGVector<index_t> idx_train =
			m_splitting_strategy->generate_subset_inverse(i);

		SGVector<index_t> idx_test =
			m_splitting_strategy->generate_subset_indices(i);

		auto features_train = view(m_features, idx_train);
		auto labels_train = view(m_labels, idx_train);
		auto features_test = view(m_features, idx_test);
		auto labels_test = view(m_labels, idx_test);
		SG_REF(features_train);
		SG_REF(labels_train);
		SG_REF(features_test);
		SG_REF(labels_test);

		auto evaluation_criterion = make_clone(m_evaluation_criterion);

		machine->set_labels(labels_train);
		machine->train(features_train);

		auto result_labels = machine->apply(features_test);
		SG_REF(result_labels);

		results[i] = evaluation_criterion->evaluate(result_labels, labels_test);
		SG_INFO("Result of cross-validation fold %d/%d is %f\n", i+1, num_subsets, results[i])

		SG_UNREF(machine);
		SG_UNREF(features_train);
		SG_UNREF(labels_train);
		SG_UNREF(features_test);
		SG_UNREF(labels_test);
		SG_UNREF(evaluation_criterion);
		SG_UNREF(result_labels);
	}

	/* build arithmetic mean of results */
	float64_t mean = CStatistics::mean(results);

	SG_DEBUG("leaving %s::evaluate_one_run()\n", get_name())
	return mean;
}
