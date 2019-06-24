/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Thoralf Klein, Heiko Strathmann, Bjoern Esser, 
 *          Evangelos Anagnostopoulos
 */

#include <gtest/gtest.h>
#include <shogun/features/DenseFeatures.h>
#include <shogun/features/CombinedFeatures.h>
#include <shogun/mathematics/NormalDistribution.h>

#include <random>

using namespace shogun;

TEST(CombinedFeaturesTest,test_array_operations)
{
	index_t n_1=3;
	index_t dim=2;

	SGMatrix<float64_t> data_1(dim,n_1);
	SGMatrix<float64_t> data_2(dim,n_1);
	SGMatrix<float64_t> data_3(dim,n_1);
	for (index_t i=0; i < dim*n_1; i++)
	{
		data_1[i] = i;
		data_2[i] = -i;
		data_3[i] = 2*i;
	}

	CCombinedFeatures* comb_feat = new CCombinedFeatures();
	CDenseFeatures<float64_t>* feat_1 = new CDenseFeatures<float64_t>(data_1);
	CDenseFeatures<float64_t>* feat_2 = new CDenseFeatures<float64_t>(data_2);
	CDenseFeatures<float64_t>* feat_3 = new CDenseFeatures<float64_t>(data_3);

	if (comb_feat->append_feature_obj(feat_1))
		EXPECT_EQ(comb_feat->get_num_feature_obj(),1);

	if (comb_feat->append_feature_obj(feat_2))
		EXPECT_EQ(comb_feat->get_num_feature_obj(),2);

	if (comb_feat->insert_feature_obj(feat_3, 1))
		EXPECT_EQ(comb_feat->get_num_feature_obj(),3);

	comb_feat->delete_feature_obj(0);
	EXPECT_EQ(comb_feat->get_num_feature_obj(),2);

	CDenseFeatures<float64_t>* f_1 = (CDenseFeatures<float64_t>*) comb_feat->get_feature_obj(0);
	SGMatrix<float64_t> m_1 = f_1->get_feature_matrix();
	CDenseFeatures<float64_t>* f_2 = (CDenseFeatures<float64_t>*) comb_feat->get_feature_obj(1);
	SGMatrix<float64_t> m_2 = f_2->get_feature_matrix();
	for (index_t i=0; i < dim*n_1; i++)
	{
		EXPECT_EQ(m_1[i], data_3[i]);
		EXPECT_EQ(m_2[i], data_2[i]);
	}
	SG_UNREF(f_1);
	SG_UNREF(f_2);

	SG_UNREF(comb_feat);
}

TEST(CombinedFeaturesTest,create_merged_copy)
{
	/* create two matrices, feature objects for them, call create_merged_copy,
	 * and check if it worked */
	int32_t seed = 100;
	index_t n_1=3;
	index_t n_2=4;
	index_t dim=2;

	SGMatrix<float64_t> data_1(dim,n_1);
	for (index_t i=0; i<dim*n_1; ++i)
		data_1.matrix[i]=i;

//	data_1.display_matrix("data_1");
	std::mt19937_64 prng(seed);
	NormalDistribution<float64_t> normal_dist;
	SGMatrix<float64_t> data_2(dim,n_2);
	for (index_t i=0; i<dim*n_2; ++i)
		data_2.matrix[i]=normal_dist(prng);

//	data_1.display_matrix("data_2");

	CCombinedFeatures* features_1=new CCombinedFeatures();
	CCombinedFeatures* features_2=new CCombinedFeatures();

	features_1->append_feature_obj(new CDenseFeatures<float64_t>(data_1));
	features_2->append_feature_obj(new CDenseFeatures<float64_t>(data_2));

	CFeatures* concatenation=features_1->create_merged_copy(features_2);

	CFeatures* sub=((CCombinedFeatures*)concatenation)->get_first_feature_obj();
	CDenseFeatures<float64_t>* casted_sub=
			dynamic_cast<CDenseFeatures<float64_t>*>(sub);
	ASSERT(casted_sub);
	SGMatrix<float64_t> concat_data=casted_sub->get_feature_matrix();
	SG_UNREF(sub);
//	concat_data.display_matrix("concat_data");

	/* check for equality with data_1 */
	for (index_t i=0; i<dim*n_1; ++i)
		EXPECT_EQ(data_1.matrix[i], concat_data.matrix[i]);

	/* check for equality with data_2 */
	for (index_t i=0; i<dim*n_2; ++i)
		EXPECT_EQ(data_2.matrix[i], concat_data.matrix[n_1*dim+i]);

	SG_UNREF(concatenation);
	SG_UNREF(features_1);
	SG_UNREF(features_2);
}
