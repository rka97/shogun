/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Heiko Strathmann
 */
#include <gtest/gtest.h>
#include <shogun/kernel/string/CommUlongStringKernel.h>
#include <shogun/kernel/normalizer/IdentityKernelNormalizer.h>
#include <shogun/preprocessor/SortUlongString.h>
#include <shogun/lib/NGramTokenizer.h>
#include <shogun/features/hashed/HashedDocDotFeatures.h>

using namespace shogun;

TEST(CommUlongStringKernel, kernel_matrix)
{
	const char* doc_1 = "stringkernelngram1";
	const char* doc_2 = "kernelngram1string";
	const char* doc_3 = "nrgam1kernelstring";

	SGVector<char> string_1(65);
	for (index_t i=0; i<65; i++)
		string_1[i] = doc_1[i];

	SGVector<char> string_2(72);
	for (index_t i=0; i<72; i++)
		string_2[i] = doc_2[i];

	SGVector<char> string_3(85);
	for (index_t i=0; i<85; i++)
		string_3[i] = doc_3[i];

	std::vector<SGVector<char>> list;
	list.reserve(3);
	list.push_back(string_1);
	list.push_back(string_2);
	list.push_back(string_3);

	auto s_feats = some<CStringFeatures<char>>(list, RAWBYTE);

	auto alphabet = s_feats->get_alphabet();
	auto l_feats = some<CStringFeatures<uint64_t>>(alphabet);
	l_feats->obtain_from_char(s_feats, 5-1, 5, 0, false);
	auto preproc = some<CSortUlongString>();
	preproc->fit(l_feats);
	l_feats =
	    wrap(preproc->transform(l_feats)->as<CStringFeatures<uint64_t>>());

	auto kernel = some<CCommUlongStringKernel>(l_feats, l_feats);
	auto normalizer = some<CIdentityKernelNormalizer>();
	kernel->set_normalizer(normalizer);


	auto h_feats =
	    some<CHashedDocDotFeatures>(20, s_feats, new CNGramTokenizer(5), false);

	SGMatrix<float64_t> kernel_matrix = kernel->get_kernel_matrix();

	SGMatrix<float64_t> feat_matrix(3,3);

	for (index_t i=0; i<3; i++)
	{
		for (index_t j=0; j<3; j++)
			feat_matrix(i,j) = h_feats->dot(i, h_feats, j);
	}

	for (index_t i=0; i<3; i++)
	{
		for (index_t j=0; j<3; j++)
			EXPECT_EQ(feat_matrix(i,j), kernel_matrix(i,j));
	}
}
