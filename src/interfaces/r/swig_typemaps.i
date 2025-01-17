/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Sergey Lisitsyn
 */

%{

#include <shogun/lib/DataType.h>
#include <shogun/lib/memory.h>

extern "C" {
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include <R_ext/Rdynload.h>
#include <Rembedded.h>
#include <Rinterface.h>
#include <R_ext/RS.h>
#include <R_ext/Error.h>
}

/* workaround compile bug in R-modular interface */
#ifndef ScalarReal
#define ScalarReal      Rf_ScalarReal
#endif

%}

/* One dimensional input arrays */
%define TYPEMAP_IN_SGVECTOR(r_type, r_cast, sg_type, error_string)
%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER)
    shogun::SGVector<sg_type>
{
    $1 = (TYPEOF($input) == r_type && Rf_ncols($input)==1 ) ? 1 : 0;
}

%typemap(in) shogun::SGVector<sg_type>
{
    SEXP rvec=$input;
    if (TYPEOF(rvec) != r_type || Rf_ncols(rvec)!=1)
    {
        /*SG_ERROR("Expected Double Vector as argument %d\n", m_rhs_counter);*/
        SWIG_fail;
    }

    $1 = shogun::SGVector<sg_type>((sg_type*) get_copy(r_cast(rvec), sizeof(sg_type)*LENGTH(rvec)), LENGTH(rvec));
}
%typemap(freearg) shogun::SGVector<sg_type>
{
}
%enddef

TYPEMAP_IN_SGVECTOR(INTSXP, INTEGER, int32_t, "Integer")
TYPEMAP_IN_SGVECTOR(REALSXP, REAL, float64_t, "Double Precision")
#undef TYPEMAP_IN_SGVECTOR

/* One dimensional output arrays */
%define TYPEMAP_OUT_SGVECTOR(r_type, r_cast, r_type_string, sg_type, if_type, error_string)
%typemap(out) shogun::SGVector<sg_type>
{
    sg_type* vec = $1.vector;
    int32_t len = $1.vlen;

    Rf_protect( $result = Rf_allocVector(r_type, len) );

    for (int32_t i=0; i<len; i++)
        r_cast($result)[i]=(if_type) vec[i];

    Rf_unprotect(1);
}

%typemap("rtype") shogun::SGVector<sg_type>   r_type_string

%typemap("scoerceout") shogun::SGVector<sg_type>
%{ %}

%enddef

TYPEMAP_OUT_SGVECTOR(INTSXP, INTEGER, "integer", uint8_t, int, "Byte")
TYPEMAP_OUT_SGVECTOR(INTSXP, INTEGER, "integer", int32_t, int, "Integer")
TYPEMAP_OUT_SGVECTOR(INTSXP, INTEGER, "integer", int16_t, int, "Short")
TYPEMAP_OUT_SGVECTOR(REALSXP, REAL, "numeric", float32_t, float, "Single Precision")
TYPEMAP_OUT_SGVECTOR(REALSXP, REAL, "numeric", float64_t, double, "Double Precision")
TYPEMAP_OUT_SGVECTOR(INTSXP, INTEGER, "integer", uint16_t, int, "Word")

#undef TYPEMAP_OUT_SGVECTOR

%define TYPEMAP_IN_SGMATRIX(r_type, r_cast, sg_type, error_string)
%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER)
        shogun::SGMatrix<sg_type>
{

    $1 = (TYPEOF($input) == r_type) ? 1 : 0;
}

%typemap(in) shogun::SGMatrix<sg_type>
{
    if( TYPEOF($input) != r_type)
    {
        /*SG_ERROR("Expected Double Matrix as argument %d\n", m_rhs_counter);*/
        SWIG_fail;
    }

    $1 = shogun::SGMatrix<sg_type>((sg_type*) get_copy(r_cast($input), ((size_t) Rf_nrows($input))*Rf_ncols($input)*sizeof(sg_type)), Rf_nrows($input), Rf_ncols($input));
}
%typemap(freearg) shogun::SGMatrix<sg_type>
{
}
%enddef

TYPEMAP_IN_SGMATRIX(INTSXP, INTEGER, int32_t, "Integer")
TYPEMAP_IN_SGMATRIX(REALSXP, REAL, float64_t, "Double Precision")
#undef TYPEMAP_IN_SGMATRIX

%define TYPEMAP_OUT_SGMATRIX(r_type, r_cast, sg_type, if_type, error_string)
%typemap(out) shogun::SGMatrix<sg_type>
{
    sg_type* matrix = $1.matrix;
    int32_t num_feat = $1.num_rows;
    int32_t num_vec = $1.num_cols;

    Rf_protect( $result = Rf_allocMatrix(r_type, num_feat, num_vec) );

    for (int32_t i=0; i<num_vec; i++)
    {
        for (int32_t j=0; j<num_feat; j++)
            r_cast($result)[i*num_feat+j]=(if_type) matrix[i*num_feat+j];
    }

    Rf_unprotect(1);
}

%typemap("rtype") shogun::SGMatrix<sg_type>   "matrix"

%typemap("scoerceout") shogun::SGMatrix<sg_type>
%{ %}

%enddef

TYPEMAP_OUT_SGMATRIX(INTSXP, INTEGER, uint8_t, int, "Byte")
TYPEMAP_OUT_SGMATRIX(INTSXP, INTEGER, int32_t, int, "Integer")
TYPEMAP_OUT_SGMATRIX(INTSXP, INTEGER, int16_t, int, "Short")
TYPEMAP_OUT_SGMATRIX(REALSXP, REAL, float32_t, float, "Single Precision")
TYPEMAP_OUT_SGMATRIX(REALSXP, REAL, float64_t, double, "Double Precision")
TYPEMAP_OUT_SGMATRIX(INTSXP, INTEGER, uint16_t, int, "Word")
#undef TYPEMAP_OUT_SGMATRIX

/* TODO INND ARRAYS */

/* input typemap for CStringFeatures<char> etc */
%define TYPEMAP_STRINGFEATURES_IN(r_type, sg_type, if_type, error_string)
%typemap(in) std::vector<shogun::SGVector<sg_type>>
{
    std::vector<shogun::SGVector<sg_type>>& strs = $1;

    int32_t max_len=0;
    int32_t num_strings=0;

    if ($input == R_NilValue || TYPEOF($input) != STRSXP)
    {
        /* SG_ERROR("Expected String List as argument %d\n", m_rhs_counter);*/
        SWIG_fail;
    }

    num_strings=Rf_length($input);
    ASSERT(num_strings>=1);
    strs.reserve(num_strings);

    for (int32_t i=0; i<num_strings; i++)
    {
        SEXPREC* s= STRING_ELT($input,i);
        sg_type* c= (sg_type*) if_type(s);
        int32_t len=LENGTH(s);
        strs.emplace_back(len+1);
        strs.back().vlen = len;
        if (len>0)
        {
			sg_memcpy(strs.back().vector, c, len*sizeof(sg_type));
            strs.back().vector[len]='\0'; /* zero terminate */
        }
        else
        {
            /*SG_WARNING( "string with index %d has zero length.\n", i+1);*/
        }
    }
}
%enddef

TYPEMAP_STRINGFEATURES_IN(STRSXP, char, CHAR, "Char")
#undef TYPEMAP_STRINGFEATURES_IN

/* TODO STRING OUT TYPEMAPS */
