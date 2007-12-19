/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2007 Vojtech Franc 
 * Written (W) 2007 Soeren Sonnenburg
 * Copyright (C) 2007 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#include "features/Labels.h"
#include "lib/Mathematics.h"
#include "lib/DynamicArray.h"
#include "lib/Time.h"
#include "base/Parallel.h"
#include "classifier/Classifier.h"
#include "classifier/svm/libocas.h"
#include "classifier/svm/WDSVMOcas.h"
#include "features/StringFeatures.h"
#include "features/Labels.h"

CWDSVMOcas::CWDSVMOcas(E_SVM_TYPE type) : CClassifier(), use_bias(false), bufsize(3000), C1(1), C2(1),
	epsilon(1e-3), method(type)
{
	w=NULL;
	old_w=NULL;
}

CWDSVMOcas::CWDSVMOcas(DREAL C, CStringFeatures<BYTE>* traindat, CLabels* trainlab) 
: CClassifier(), use_bias(false), bufsize(3000), C1(C), C2(C), epsilon(1e-3)
{
	w=NULL;
	old_w=NULL;
	method=SVM_OCAS;
	features=traindat;
	CClassifier::labels=trainlab;
}


CWDSVMOcas::~CWDSVMOcas()
{
}

bool CWDSVMOcas::train()
{
	SG_INFO("C=%f, epsilon=%f, bufsize=%d\n", get_C1(), get_epsilon(), bufsize);

	ASSERT(get_labels());
	ASSERT(get_features());
	ASSERT(get_labels()->is_two_class_labeling());

	string_length=get_features()->get_max_vector_length();
	INT num_train_labels=0;
	lab=get_labels()->get_labels(num_train_labels);

	for (INT i=0; i<degree; i++)
		wd_weights[i]=2*(degree-i)/(degree*(degree));

	w_dim_single_char=((CMath::pow(alphabet_size,degree+1)-1)/(CMath::pow(alphabet_size,degree)-1)-1);
	w_dim=string_length*w_dim_single_char;
	SG_DEBUG("cutting plane has %d dims\n", w_dim);
	INT num_vec=features->get_num_vectors();

	ASSERT(num_vec==num_train_labels);
	ASSERT(num_vec>0);

	delete[] w;
	w=new DREAL[w_dim];
	ASSERT(w);
	memset(w, 0, w_dim*sizeof(DREAL));

	delete[] old_w;
	old_w=new DREAL[w_dim];
	ASSERT(old_w);
	memset(old_w, 0, w_dim*sizeof(DREAL));
	bias=0;

	cuts=new DREAL*[bufsize];
	ASSERT(cuts);
	memset(cuts, 0, sizeof(*cuts)*bufsize);

	double TolAbs=0;
	double QPBound=0;
	int Method=0;
	if (method == SVM_OCAS)
		Method = 1;
	ocas_return_value_T result = svm_ocas_solver( get_C1(), num_vec, get_epsilon(),
			TolAbs, QPBound, bufsize, Method, 
			&CWDSVMOcas::compute_W,
			&CWDSVMOcas::update_W, 
			&CWDSVMOcas::add_new_cut, 
			&CWDSVMOcas::compute_output,
			&CWDSVMOcas::sort,
			&printf,
			this);

	for (INT i=bufsize-1; i>=0; i--)
		delete[] cuts[i];
	delete[] cuts;

	delete[] lab;
	lab=NULL;

	return true;
}

/*----------------------------------------------------------------------------------
  sq_norm_W = sparse_update_W( t ) does the following:

  W = oldW*(1-t) + t*W;
  sq_norm_W = W'*W;

  ---------------------------------------------------------------------------------*/
double CWDSVMOcas::update_W( double t, void* ptr )
{
  double sq_norm_W = 0;         
  CWDSVMOcas* o = (CWDSVMOcas*) ptr;
  uint32_t nDim = (uint32_t) o->w_dim;
  double* W=o->w;
  double* oldW=o->old_w;

  for(uint32_t j=0; j <nDim; j++)
  {
	  W[j] = oldW[j]*(1-t) + t*W[j];
	  sq_norm_W += W[j]*W[j];
  }          

  return( sq_norm_W );
}

/*----------------------------------------------------------------------------------
  sparse_add_new_cut( new_col_H, new_cut, cut_length, nSel ) does the following:

    new_a = sum(data_X(:,find(new_cut ~=0 )),2);
    new_col_H = [sparse_A(:,1:nSel)'*new_a ; new_a'*new_a];
    sparse_A(:,nSel+1) = new_a;

  ---------------------------------------------------------------------------------*/
void CWDSVMOcas::add_new_cut( double *new_col_H, 
                  uint32_t *new_cut, 
                  uint32_t cut_length, 
                  uint32_t nSel,
				  void* ptr)
{
	CWDSVMOcas* o = (CWDSVMOcas*) ptr;
	CStringFeatures<BYTE>* f = o->features;
	INT string_length = o->string_length;
	uint32_t nDim=(uint32_t) o->w_dim;
	DREAL* y = o->lab;
	INT alphabet_size = o->alphabet_size;
	INT w_dim_single_char = o->w_dim_single_char;
	DREAL* wd_weights = o->wd_weights;
	INT degree = o->degree;
	double** cuts=o->cuts;

	uint32_t i;

	// temporary vector
	double* new_a = new DREAL[nDim];
	memset(new_a, 0, sizeof(double)*nDim);

	for(i=0; i < cut_length; i++) 
	{
		INT offs=0;
		INT len=0;
		BYTE* vec = f->get_feature_vector(new_cut[i], len);
		ASSERT(len == string_length);
		DREAL scalar=y[new_cut[i]];

		for (INT j=0; j<len; j++)
		{
			INT val=0;
			for (INT k=0; (j+k<string_length) && (k<degree); k++)
			{
				val=val*alphabet_size + vec[j+k];
				new_a[offs+val]+=wd_weights[k] * scalar;
			}
			offs+=w_dim_single_char;
		}
	}

	// insert new_a into the last column of sparse_A
	for(i=0; i < nSel; i++)
		new_col_H[i] = CMath::dot(new_a, cuts[i], nDim);
	new_col_H[nSel] = CMath::dot(new_a, new_a, nDim);

	cuts[nSel]=new_a;
}

void CWDSVMOcas::sort( double* vals, uint32_t* idx, uint32_t size)
{
	CMath::qsort_index(vals, idx, size);
}

/*----------------------------------------------------------------------
  sparse_compute_output( output ) does the follwing:

  output = data_X'*W;
  ----------------------------------------------------------------------*/
void CWDSVMOcas::compute_output( double *output, void* ptr )
{
	CWDSVMOcas* o = (CWDSVMOcas*) ptr;
	CStringFeatures<BYTE>* f=o->get_features();
	INT nData=f->get_num_vectors();

	DREAL* y = o->lab;
	INT degree = o->degree;
	INT string_length = o->string_length;
	INT alphabet_size = o->alphabet_size;
	INT w_dim_single_char = o->w_dim_single_char;
	DREAL* wd_weights = o->wd_weights;
	DREAL* w= o->w;
	INT len;

	for (INT i=0; i<nData; i++)
	{
		DREAL sum=0;
		INT offs=0;

		BYTE* vec = f->get_feature_vector(i, len);
		ASSERT(len == string_length);

		for (INT j=0; j<string_length; j++)
		{
			INT val=0;
			for (INT k=0; (j+k<string_length) && (k<degree); k++)
			{
				val=val*alphabet_size + vec[j+k];
				sum+=wd_weights[k] * w[offs+val];
			}
			offs+=w_dim_single_char;
		}
		output[i]=y[i]*sum;
	}
}
/*----------------------------------------------------------------------
  sq_norm_W = compute_W( alpha, nSel ) does the following:

  oldW = W;
  W = sparse_A(:,1:nSel)'*alpha;
  sq_norm_W = W'*W;
  dp_WoldW = W'*oldW';

  ----------------------------------------------------------------------*/
void CWDSVMOcas::compute_W( double *sq_norm_W, double *dp_WoldW, double *alpha, uint32_t nSel, void* ptr )
{
	CWDSVMOcas* o = (CWDSVMOcas*) ptr;
	uint32_t nDim= (uint32_t) o->w_dim;
	CMath::swap(o->w, o->old_w);
	double* W=o->w;
	double** cuts=o->cuts;
	double* oldW=o->old_w;
	memset(W, 0, sizeof(double)*nDim);

	for (uint32_t i=0; i<nSel; i++)
	{
		if (alpha[i] > 0)
			CMath::vec1_plus_scalar_times_vec2(W, alpha[i], cuts[i], nDim);
	}

	*sq_norm_W = CMath::dot(W,W, nDim);
	*dp_WoldW = CMath::dot(W,oldW, nDim);;
}
