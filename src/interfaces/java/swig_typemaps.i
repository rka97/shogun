/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Sergey Lisitsyn
 */
 
/*
 * NOTE: Disabled boolean vector and matrix typemaps due to jblas is not able
 * to represent them.
 * Once we use a proper matrix library, can re-enable them again.
 * See github issue 3554
 * Heiko Strathmann
 */

%include <java/enumtypeunsafe.swg>

#ifdef HAVE_JBLAS
%pragma(java) jniclassimports=%{
import org.jblas.*;
%}
%typemap(javaimports) SWIGTYPE%{
import java.io.Serializable;
import org.jblas.*;
%}
#else
#ifdef HAVE_UJMP
%pragma(java) jniclassimports=%{
import org.ujmp.core.*;
import org.ujmp.core.doublematrix.impl.DefaultDenseDoubleMatrix2D;
import org.ujmp.core.floatmatrix.impl.DefaultDenseFloatMatrix2D;
import org.ujmp.core.intmatrix.impl.DefaultDenseIntMatrix2D;
import org.ujmp.core.longmatrix.impl.DefaultDenseLongMatrix2D;
import org.ujmp.core.shortmatrix.impl.DefaultDenseShortMatrix2D;
import org.ujmp.core.bytematrix.impl.DefaultDenseByteMatrix2D;
import org.ujmp.core.booleanmatrix.impl.DefaultDenseBooleanMatrix2D;
%}
%typemap(javaimports) SWIGTYPE%{
import org.ujmp.core.*;
import org.ujmp.core.doublematrix.impl.DefaultDenseDoubleMatrix2D;
import org.ujmp.core.floatmatrix.impl.DefaultDenseFloatMatrix2D;
import org.ujmp.core.intmatrix.impl.DefaultDenseIntMatrix2D;
import org.ujmp.core.longmatrix.impl.DefaultDenseLongMatrix2D;
import org.ujmp.core.shortmatrix.impl.DefaultDenseShortMatrix2D;
import org.ujmp.core.bytematrix.impl.DefaultDenseByteMatrix2D;
import org.ujmp.core.booleanmatrix.impl.DefaultDenseBooleanMatrix2D;
%}
#endif
#endif
/* One dimensional input/output arrays */
#ifdef HAVE_JBLAS
/* Two dimensional input/output arrays */
%define TYPEMAP_SGVECTOR(SGTYPE, JTYPE, JAVATYPE, JNITYPE, TOARRAY, CLASSDESC, CONSTRUCTOR)

%typemap(jni) shogun::SGVector<SGTYPE>		%{jobject%}
%typemap(jtype) shogun::SGVector<SGTYPE>		%{##JAVATYPE##Matrix%}
%typemap(jstype) shogun::SGVector<SGTYPE>	%{##JAVATYPE##Matrix%}

%typemap(in) shogun::SGVector<SGTYPE>
{
	jclass cls;
	jmethodID mid;
	SGTYPE *array;
	##JNITYPE##Array jarr;
	JNITYPE *carr;
	int32_t i, cols;
	bool isVector;

	if (!$input) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
		return $null;
	}

	cls = JCALL1(GetObjectClass, jenv, $input);
	if (!cls)
		return $null;

	mid = JCALL3(GetMethodID, jenv, cls, "isVector", "()Z");
	if (!mid)
		return $null;

	isVector = (int32_t)JCALL2(CallIntMethod, jenv, $input, mid);
	if (!isVector) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "vector expected");
		return $null;
	}

	mid = JCALL3(GetMethodID, jenv, cls, "getColumns", "()I");
	if (!mid)
		return $null;

	cols = (int32_t)JCALL2(CallIntMethod, jenv, $input, mid);
	if (cols < 1) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null vector");
		return $null;
	}

	mid = JCALL3(GetMethodID, jenv, cls, "toArray", TOARRAY);
	if (!mid)
		return $null;

	jarr = (##JNITYPE##Array)JCALL2(CallObjectMethod, jenv, $input, mid);
	carr = JCALL2(Get##JAVATYPE##ArrayElements, jenv, jarr, 0);
	array = SG_MALLOC(SGTYPE, cols);
	for (i = 0; i < cols; i++) {
		array[i] = (SGTYPE)carr[i];
	}

	JCALL3(Release##JAVATYPE##ArrayElements, jenv, jarr, carr, 0);

    $1 = shogun::SGVector<SGTYPE>((SGTYPE *)array, cols);
}

%typemap(out) shogun::SGVector<SGTYPE>
{
	int32_t rows = 1;
	int32_t cols = $1.vlen;
	JNITYPE* arr = SG_MALLOC(JNITYPE,cols);
	jobject res;
	int32_t i;

	jclass cls;
	jmethodID mid;

	cls = JCALL1(FindClass, jenv, CLASSDESC);
	if (!cls)
		return $null;

	mid = JCALL3(GetMethodID, jenv, cls, "<init>", CONSTRUCTOR);
	if (!mid)
		return $null;

	##JNITYPE##Array jarr = (##JNITYPE##Array)JCALL1(New##JAVATYPE##Array, jenv, cols);
	if (!jarr)
		return $null;

	for (i = 0; i < cols; i++) {
		arr[i] = (JNITYPE)$1.vector[i];
	}

	JCALL4(Set##JAVATYPE##ArrayRegion, jenv, jarr, 0, cols, arr);

	res = JCALL5(NewObject, jenv, cls, mid, rows, cols, jarr);
        SG_FREE(arr);
	$result = (jobject)res;
}

%typemap(javain) shogun::SGVector<SGTYPE> "$javainput"
%typemap(javaout) shogun::SGVector<SGTYPE> {
	return $jnicall;
}

%enddef

/*Define concrete examples of the TYPEMAP_SGVector macros */
//TYPEMAP_SGVECTOR(bool, boolean, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(char, byte, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(uint8_t, byte, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(int16_t, short, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(uint16_t, int, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(int32_t, int, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(uint32_t, long, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(int64_t, int, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(uint64_t, long, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(long long, long, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGVECTOR(float32_t, float, Float, jfloat, "()[F", "org/jblas/FloatMatrix","(II[F)V")
TYPEMAP_SGVECTOR(float64_t, double, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")

#undef TYPEMAP_SGVECTOR

#else
#ifdef HAVE_UJMP
/* Two dimensional input/output arrays */
%define TYPEMAP_SGVECTOR(SGTYPE, JTYPE, JAVATYPE, JNITYPE, TOARRAYMETHOD, TOARRAYDESC, CLASSDESC, CONSTRUCTOR)

%typemap(jni) shogun::SGVector<SGTYPE>		%{jobject%}
%typemap(jtype) shogun::SGVector<SGTYPE>		%{Matrix%}
%typemap(jstype) shogun::SGVector<SGTYPE>	%{Matrix%}

%typemap(in) shogun::SGVector<SGTYPE>
{
	jclass cls;
	jmethodID mid;
	SGTYPE *array;
	jobjectArray jobj;
	##JNITYPE##Array jarr;
	JNITYPE *carr;
	bool isVector;
	int32_t i, rows, cols;

	if (!$input) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
		return $null;
	}

	cls = JCALL1(GetObjectClass, jenv, $input);
	if (!cls)
		return $null;

	mid = JCALL3(GetMethodID, jenv, cls, "isRowVector", "()Z");
	if (!mid)
		return $null;

	isVector = (int32_t)JCALL2(CallIntMethod, jenv, $input, mid);
	if (!isVector) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "vector expected");
		return $null;
	}

	mid = JCALL3(GetMethodID, jenv, cls, " getColumnCount", "()I");
	if (!mid)
		return $null;

	cols = (int32_t)JCALL2(CallIntMethod, jenv, $input, mid);
	if (cols < 1) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null vector");
		return $null;
	}

	mid = JCALL3(GetMethodID, jenv, cls, TOARRAYMETHOD, TOARRAYDESC);
	if (!mid)
		return $null;

	jobj = (jobjectArray)JCALL2(CallObjectMethod, jenv, $input, mid);
	jarr = (JNITYPE##Array)JCALL2(GetObjectArrayElement, jenv, jobj, 0);
	if (!jarr) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
		return $null;
	}

	carr = JCALL2(Get##JAVATYPE##ArrayElements, jenv, jarr, 0);
	array = SG_MALLOC(SGTYPE, cols);
	if (!array) {
	SWIG_JavaThrowException(jenv, SWIG_JavaOutOfMemoryError, "array memory allocation failed");
	return $null;
	}

	for (i = 0; i < cols; i++) {
		array[j] = carr[i];
	}

	JCALL3(Release##JAVATYPE##ArrayElements, jenv, jarr, carr, 0);

	$1 = shogun::SGVector<SGTYPE>((SGTYPE *)array, cols);
}

%typemap(out) shogun::SGVector<SGTYPE>
{
	int32_t rows = 1;
	int32_t cols = $1.vlen;
	JNITYPE* arr = SG_MALLOC(JNITYPE,cols);
	jobject res;
	int32_t i;

	jclass cls;
	jmethodID mid;

	cls = JCALL1(FindClass, jenv, CLASSDESC);
	if (!cls)
		return $null;

	mid = JCALL3(GetMethodID, jenv, cls, "<init>", CONSTRUCTOR);
	if (!mid)
		return $null;

	##JNITYPE##Array jarr = (##JNITYPE##Array)JCALL1(New##JAVATYPE##Array, jenv, cols);
	if (!jarr)
		return $null;

	for (i = 0; i < cols; i++) {
		arr[i] = (JNITYPE)$1.vector[i];
	}

	JCALL4(Set##JAVATYPE##ArrayRegion, jenv, jarr, 0, cols, arr);

	res = JCALL5(NewObject, jenv, cls, mid, jarr, rows, cols);

        SG_FREE(arr);
	$result = (jobject)res;
}

%typemap(javain) shogun::SGVector<SGTYPE> "$javainput"
%typemap(javaout) shogun::SGVector<SGTYPE> {
	return $jnicall;
}

%enddef

/*Define concrete examples of the TYPEMAP_SGVECTOR macros */
//TYPEMAP_SGVECTOR(bool, boolean, Boolean, jboolean, "toBooleanArray", "()[[Z", "org/ujmp/core/booleanmatrix/impl/DefaultDenseBooleanMatrix2D", "([BII)V")
TYPEMAP_SGVECTOR(char, byte, Byte, jbyte, "toByteArray", "()[[B", "org/ujmp/core/bytematrix/impl/DefaultDenseByteMatrix2D", "([BII)V")
TYPEMAP_SGVECTOR(uint8_t, byte, Byte, jbyte, "toByteArray", "()[[B", "org/ujmp/core/bytematrix/impl/DefaultDenseByteMatrix2D", "([BII)V")
TYPEMAP_SGVECTOR(int16_t, short, Short, jshort, "toShortArray", "()[[S", "org/ujmp/core/shortmatrix/impl/DefaultDenseShortMatrix2D", "([SII)V")
TYPEMAP_SGVECTOR(uint16_t, int, Int, jint, "toIntArray", "()[[I", "org/ujmp/core/intmatrix/impl/DefaultDenseIntMatrix2D", "([III)V")
TYPEMAP_SGVECTOR(int32_t, int, Int, jint, "toIntArray", "()[[I", "org/ujmp/core/intmatrix/impl/DefaultDenseIntMatrix2D", "([III)V")
TYPEMAP_SGVECTOR(uint32_t, long, Long, jlong, "toLongArray", "()[[J", "org/ujmp/core/longmatrix/impl/DefaultDenseLongMatrix2D", "([JII)V")
TYPEMAP_SGVECTOR(int64_t, int, Int, jint, "toIntArray", "()[[I", "org/ujmp/core/intmatrix/impl/DefaultDenseIntMatrix2D", "([III)V")
TYPEMAP_SGVECTOR(uint64_t, long, Long, jlong, "toLongArray", "()[[J", "org/ujmp/core/longmatrix/impl/DefaultDenseLongMatrix2D", "([JII)V")
TYPEMAP_SGVECTOR(long long, long, Long, jlong, "toLongArray", "()[[J", "org/ujmp/core/longmatrix/impl/DefaultDenseLongMatrix2D", "([JII)V")
TYPEMAP_SGVECTOR(float32_t, float, Float, jfloat, "toFloatArray", "()[[F", "org/ujmp/core/floatmatrix/impl/DefaultDenseFloatMatrix2D", "([FII)V")
TYPEMAP_SGVECTOR(float64_t, double, Double, jdouble, "toDoubleArray", "()[[D", "org/ujmp/core/doublematrix/impl/DefaultDenseDoubleMatrix2D", "([DII)V")

#undef TYPEMAP_SGVECTOR

/* Two dimensional input/output arrays */
%define TYPEMAP_SGVECTOR_REF(SGTYPE, JTYPE, JAVATYPE, JNITYPE, TOARRAYMETHOD, TOARRAYDESC, CLASSDESC, CONSTRUCTOR)

%typemap(jni) shogun::SGVector<SGTYPE>&, const shogun::SGVector<SGTYPE>&		%{jobject%}
%typemap(jtype) shogun::SGVector<SGTYPE>&, const shogun::SGVector<SGTYPE>&		%{Matrix%}
%typemap(jstype) shogun::SGVector<SGTYPE>&, const shogun::SGVector<SGTYPE>&	%{Matrix%}

%typemap(in) shogun::SGVector<SGTYPE>& (SGVector<SGTYPE> temp), const shogun::SGVector<SGTYPE>& (SGVector<SGTYPE> temp)
{
	jclass cls;
	jmethodID mid;
	SGTYPE *array;
	jobjectArray jobj;
	##JNITYPE##Array jarr;
	JNITYPE *carr;
	bool isVector;
	int32_t i, rows, cols;

	if (!$input) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
		return $null;
	}

	cls = JCALL1(GetObjectClass, jenv, $input);
	if (!cls)
		return $null;

	mid = JCALL3(GetMethodID, jenv, cls, "isRowVector", "()Z");
	if (!mid)
		return $null;

	isVector = (int32_t)JCALL2(CallIntMethod, jenv, $input, mid);
	if (!isVector) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "vector expected");
		return $null;
	}

	mid = JCALL3(GetMethodID, jenv, cls, " getColumnCount", "()I");
	if (!mid)
		return $null;

	cols = (int32_t)JCALL2(CallIntMethod, jenv, $input, mid);
	if (cols < 1) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null vector");
		return $null;
	}

	mid = JCALL3(GetMethodID, jenv, cls, TOARRAYMETHOD, TOARRAYDESC);
	if (!mid)
		return $null;

	jobj = (jobjectArray)JCALL2(CallObjectMethod, jenv, $input, mid);
	jarr = (JNITYPE##Array)JCALL2(GetObjectArrayElement, jenv, jobj, 0);
	if (!jarr) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
		return $null;
	}

	carr = JCALL2(Get##JAVATYPE##ArrayElements, jenv, jarr, 0);
	array = SG_MALLOC(SGTYPE, cols);
	if (!array) {
	SWIG_JavaThrowException(jenv, SWIG_JavaOutOfMemoryError, "array memory allocation failed");
	return $null;
	}

	for (i = 0; i < cols; i++) {
		array[j] = carr[i];
	}

	JCALL3(Release##JAVATYPE##ArrayElements, jenv, jarr, carr, 0);

	temp = shogun::SGVector<SGTYPE>((SGTYPE *)array, cols);
	$1 = &temp;
}

%typemap(javain) shogun::SGVector<SGTYPE>&, const shogun::SGVector<SGTYPE>& "$javainput"

%enddef

/*Define concrete examples of the TYPEMAP_SGVECTOR_REF macros */
//TYPEMAP_SGVECTOR_REF(bool, boolean, Boolean, jboolean, "toBooleanArray", "()[[Z", "org/ujmp/core/booleanmatrix/impl/DefaultDenseBooleanMatrix2D", "([BII)V")
TYPEMAP_SGVECTOR_REF(char, byte, Byte, jbyte, "toByteArray", "()[[B", "org/ujmp/core/bytematrix/impl/DefaultDenseByteMatrix2D", "([BII)V")
TYPEMAP_SGVECTOR_REF(uint8_t, byte, Byte, jbyte, "toByteArray", "()[[B", "org/ujmp/core/bytematrix/impl/DefaultDenseByteMatrix2D", "([BII)V")
TYPEMAP_SGVECTOR_REF(int16_t, short, Short, jshort, "toShortArray", "()[[S", "org/ujmp/core/shortmatrix/impl/DefaultDenseShortMatrix2D", "([SII)V")
TYPEMAP_SGVECTOR_REF(uint16_t, int, Int, jint, "toIntArray", "()[[I", "org/ujmp/core/intmatrix/impl/DefaultDenseIntMatrix2D", "([III)V")
TYPEMAP_SGVECTOR_REF(int32_t, int, Int, jint, "toIntArray", "()[[I", "org/ujmp/core/intmatrix/impl/DefaultDenseIntMatrix2D", "([III)V")
TYPEMAP_SGVECTOR_REF(uint32_t, long, Long, jlong, "toLongArray", "()[[J", "org/ujmp/core/longmatrix/impl/DefaultDenseLongMatrix2D", "([JII)V")
TYPEMAP_SGVECTOR_REF(int64_t, int, Int, jint, "toIntArray", "()[[I", "org/ujmp/core/intmatrix/impl/DefaultDenseIntMatrix2D", "([III)V")
TYPEMAP_SGVECTOR_REF(uint64_t, long, Long, jlong, "toLongArray", "()[[J", "org/ujmp/core/longmatrix/impl/DefaultDenseLongMatrix2D", "([JII)V")
TYPEMAP_SGVECTOR_REF(long long, long, Long, jlong, "toLongArray", "()[[J", "org/ujmp/core/longmatrix/impl/DefaultDenseLongMatrix2D", "([JII)V")
TYPEMAP_SGVECTOR_REF(float32_t, float, Float, jfloat, "toFloatArray", "()[[F", "org/ujmp/core/floatmatrix/impl/DefaultDenseFloatMatrix2D", "([FII)V")
TYPEMAP_SGVECTOR_REF(float64_t, double, Double, jdouble, "toDoubleArray", "()[[D", "org/ujmp/core/doublematrix/impl/DefaultDenseDoubleMatrix2D", "([DII)V")

#undef TYPEMAP_SGVECTOR_REF

#endif
#endif

#ifdef HAVE_JBLAS
/* Two dimensional input/output arrays */
%define TYPEMAP_SGMATRIX(SGTYPE, JTYPE, JAVATYPE, JNITYPE, TOARRAY, CLASSDESC, CONSTRUCTOR)

%typemap(jni) shogun::SGMatrix<SGTYPE>		%{jobject%}
%typemap(jtype) shogun::SGMatrix<SGTYPE>		%{##JAVATYPE##Matrix%}
%typemap(jstype) shogun::SGMatrix<SGTYPE>	%{##JAVATYPE##Matrix%}

%typemap(in) shogun::SGMatrix<SGTYPE>
{
	jclass cls;
	jmethodID mid;
	SGTYPE *array;
	##JNITYPE##Array jarr;
	JNITYPE *carr;
	int32_t i,len, rows, cols;

	if (!$input) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
		return $null;
	}

	cls = JCALL1(GetObjectClass, jenv, $input);
	if (!cls)
		return $null;

	mid = JCALL3(GetMethodID, jenv, cls, "toArray", TOARRAY);
	if (!mid)
		return $null;

	jarr = (##JNITYPE##Array)JCALL2(CallObjectMethod, jenv, $input, mid);
	carr = JCALL2(Get##JAVATYPE##ArrayElements, jenv, jarr, 0);
	len = JCALL1(GetArrayLength, jenv, jarr);
	array = SG_MALLOC(SGTYPE, len);
	for (i = 0; i < len; i++) {
		array[i] = (SGTYPE)carr[i];
	}

	JCALL3(Release##JAVATYPE##ArrayElements, jenv, jarr, carr, 0);

	mid = JCALL3(GetMethodID, jenv, cls, "getRows", "()I");
	if (!mid)
		return $null;

	rows = (int32_t)JCALL2(CallIntMethod, jenv, $input, mid);

	mid = JCALL3(GetMethodID, jenv, cls, "getColumns", "()I");
	if (!mid)
		return $null;

	cols = (int32_t)JCALL2(CallIntMethod, jenv, $input, mid);

	$1 = shogun::SGMatrix<SGTYPE>((SGTYPE*)array, rows, cols, true);
}

%typemap(out) shogun::SGMatrix<SGTYPE>
{
	int32_t rows = $1.num_rows;
	int32_t cols = $1.num_cols;
	int64_t len = int64_t(rows) * cols;
	JNITYPE* arr = SG_MALLOC(JNITYPE, len);
	jobject res;

	jclass cls;
	jmethodID mid;

	cls = JCALL1(FindClass, jenv, CLASSDESC);
	if (!cls)
		return $null;

	mid = JCALL3(GetMethodID, jenv, cls, "<init>", CONSTRUCTOR);
	if (!mid)
		return $null;

	##JNITYPE##Array jarr = (##JNITYPE##Array)JCALL1(New##JAVATYPE##Array, jenv, len);
	if (!jarr)
		return $null;

	for (int64_t i = 0; i < len; i++) {
		arr[i] = (JNITYPE)$1.matrix[i];
	}
	JCALL4(Set##JAVATYPE##ArrayRegion, jenv, jarr, 0, len, arr);

	res = JCALL5(NewObject, jenv, cls, mid, rows, cols, jarr);
        SG_FREE(arr);
	$result = (jobject)res;
}

%typemap(javain) shogun::SGMatrix<SGTYPE> "$javainput"
%typemap(javaout) shogun::SGMatrix<SGTYPE> {
	return $jnicall;
}

%enddef

/*Define concrete examples of the TYPEMAP_SGMATRIX macros */
//TYPEMAP_SGMATRIX(bool, boolean, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(char, byte, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(uint8_t, byte, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(int16_t, short, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(uint16_t, int, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(int32_t, int, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(uint32_t, long, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(int64_t, int, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(uint64_t, long, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(long long, long, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")
TYPEMAP_SGMATRIX(float32_t, float, Float, jfloat, "()[F", "org/jblas/FloatMatrix","(II[F)V")
TYPEMAP_SGMATRIX(float64_t, double, Double, jdouble, "()[D", "org/jblas/DoubleMatrix", "(II[D)V")

#undef TYPEMAP_SGMATRIX

#else
#ifdef HAVE_UJMP
/* Two dimensional input/output arrays */
%define TYPEMAP_SGMATRIX(SGTYPE, JTYPE, JAVATYPE, JNITYPE, TOARRAYMETHOD, TOARRAYDESC, CLASSDESC, CONSTRUCTOR)

%typemap(jni) shogun::SGMatrix<SGTYPE>		%{jobject%}
%typemap(jtype) shogun::SGMatrix<SGTYPE>		%{Matrix%}
%typemap(jstype) shogun::SGMatrix<SGTYPE>	%{Matrix%}

%typemap(in) shogun::SGMatrix<SGTYPE>
{
	jclass cls;
	jmethodID mid;
	SGTYPE *array;
	jobjectArray jobj;
	##JNITYPE##Array jarr;
	JNITYPE *carr;
	int32_t i, j, rows, cols;

	if (!$input) {
		SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
		return $null;
	}

	cls = JCALL1(GetObjectClass, jenv, $input);
	if (!cls)
		return $null;

	mid = JCALL3(GetMethodID, jenv, cls, TOARRAYMETHOD, TOARRAYDESC);
	if (!mid)
		return $null;

	jobj = (jobjectArray)JCALL2(CallObjectMethod, jenv, $input, mid);
	rows = JCALL1(GetArrayLength, jenv, jobj);
	cols = 0;

	for (i = 0; i < rows; i++) {
		jarr = (JNITYPE##Array)JCALL2(GetObjectArrayElement, jenv, jobj, i);
		if (!jarr) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
			return $null;
		}
		if (cols == 0) {
			cols = JCALL1(GetArrayLength, jenv, jarr);
			array = SG_MALLOC(SGTYPE, rows * cols);
			if (!array) {
			SWIG_JavaThrowException(jenv, SWIG_JavaOutOfMemoryError, "array memory allocation failed");
			return $null;
			}
		}
		carr = JCALL2(Get##JAVATYPE##ArrayElements, jenv, jarr, 0);
		for (j = 0; j < cols; j++) {
			array[i * cols + j] = carr[j];
		}
		JCALL3(Release##JAVATYPE##ArrayElements, jenv, jarr, carr, 0);
	}

	$1 = shogun::SGMatrix<SGTYPE>((SGTYPE*)array, rows, cols);
}

%typemap(out) shogun::SGMatrix<SGTYPE>
{
	int32_t rows = $1.num_rows;
	int32_t cols = $1.num_cols;
	int32_t len = rows * cols;
	JNITYPE* arr = SG_MALLOC(JNITYPE,len);
	jobject res;
	int32_t i;

	jclass cls;
	jmethodID mid;

	cls = JCALL1(FindClass, jenv, CLASSDESC);
	if (!cls)
		return $null;

	mid = JCALL3(GetMethodID, jenv, cls, "<init>", CONSTRUCTOR);
	if (!mid)
		return $null;

	##JNITYPE##Array jarr = (##JNITYPE##Array)JCALL1(New##JAVATYPE##Array, jenv, len);
	if (!jarr)
		return $null;

	for (i = 0; i < len; i++) {
		arr[i] = (JNITYPE)$1.matrix[i];
	}
	JCALL4(Set##JAVATYPE##ArrayRegion, jenv, jarr, 0, len, arr);

	res = JCALL5(NewObject, jenv, cls, mid, jarr, rows, cols);
        SG_FREE(arr);
	$result = (jobject)res;
}

%typemap(javain) shogun::SGMatrix<SGTYPE> "$javainput"
%typemap(javaout) shogun::SGMatrix<SGTYPE> {
	return $jnicall;
}

%enddef

/*Define concrete examples of the TYPEMAP_SGMATRIX macros */
//TYPEMAP_SGMATRIX(bool, boolean, Boolean, jboolean, "toBooleanArray", "()[[Z", "org/ujmp/core/booleanmatrix/impl/DefaultDenseBooleanMatrix2D", "([BII)V")
TYPEMAP_SGMATRIX(char, byte, Byte, jbyte, "toByteArray", "()[[B", "org/ujmp/core/bytematrix/impl/DefaultDenseByteMatrix2D", "([BII)V")
TYPEMAP_SGMATRIX(uint8_t, byte, Byte, jbyte, "toByteArray", "()[[B", "org/ujmp/core/bytematrix/impl/DefaultDenseByteMatrix2D", "([BII)V")
TYPEMAP_SGMATRIX(int16_t, short, Short, jshort, "toShortArray", "()[[S", "org/ujmp/core/shortmatrix/impl/DefaultDenseShortMatrix2D", "([SII)V")
TYPEMAP_SGMATRIX(uint16_t, int, Int, jint, "toIntArray", "()[[I", "org/ujmp/core/intmatrix/impl/DefaultDenseIntMatrix2D", "([III)V")
TYPEMAP_SGMATRIX(int32_t, int, Int, jint, "toIntArray", "()[[I", "org/ujmp/core/intmatrix/impl/DefaultDenseIntMatrix2D", "([III)V")
TYPEMAP_SGMATRIX(uint32_t, long, Long, jlong, "toLongArray", "()[[J", "org/ujmp/core/longmatrix/impl/DefaultDenseLongMatrix2D", "([JII)V")
TYPEMAP_SGMATRIX(int64_t, int, Int, jint, "toIntArray", "()[[I", "org/ujmp/core/intmatrix/impl/DefaultDenseIntMatrix2D", "([III)V")
TYPEMAP_SGMATRIX(uint64_t, long, Long, jlong, "toLongArray", "()[[J", "org/ujmp/core/longmatrix/impl/DefaultDenseLongMatrix2D", "([JII)V")
TYPEMAP_SGMATRIX(long long, long, Long, jlong, "toLongArray", "()[[J", "org/ujmp/core/longmatrix/impl/DefaultDenseLongMatrix2D", "([JII)V")
TYPEMAP_SGMATRIX(float32_t, float, Float, jfloat, "toFloatArray", "()[[F", "org/ujmp/core/floatmatrix/impl/DefaultDenseFloatMatrix2D", "([FII)V")
TYPEMAP_SGMATRIX(float64_t, double, Double, jdouble, "toDoubleArray", "()[[D", "org/ujmp/core/doublematrix/impl/DefaultDenseDoubleMatrix2D", "([DII)V")

#undef TYPEMAP_SGMATRIX

#endif
#endif

/* input/output typemap for CStringFeatures */
%define TYPEMAP_STRINGFEATURES(SGTYPE, JTYPE, JAVATYPE, JNITYPE, CLASSDESC)

%typemap(jni) std::vector<shogun::SGVector<SGTYPE>>	%{jobjectArray%}
%typemap(jtype) std::vector<shogun::SGVector<SGTYPE>>	%{JTYPE[][]%}
%typemap(jstype) std::vector<shogun::SGVector<SGTYPE>>	%{JTYPE[][]%}

%fragment(SWIG_AsVal_frag(std::vector<shogun::SGVector<SGTYPE>>), "header")
{
    int SWIG_AsVal_dec(std::vector<shogun::SGVector<SGTYPE>>)
    	(JNIEnv* jenv, const jobjectArray& input, std::vector<shogun::SGVector<SGTYPE>>& strings)
    {
		if (!input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
			return SWIG_ERROR;
		}

		int32_t size = JCALL1(GetArrayLength, jenv, input);
		strings.reserve(size);

		for (int32_t i = 0; i < size; i++) {
			##JNITYPE##Array jarr = (##JNITYPE##Array)JCALL2(GetObjectArrayElement, jenv, input, i);
			int32_t len = JCALL1(GetArrayLength, jenv, jarr);

			JNITYPE* carr = JCALL2(Get##JAVATYPE##ArrayElements, jenv, jarr, 0);
			
			strings.emplace_back(len);
			std::copy_n(carr, len, strings.back().vector);

			JCALL3(Release##JAVATYPE##ArrayElements, jenv, jarr, carr, 0);
		}
		return SWIG_OK;
    }
}

%fragment(SWIG_From_frag(std::vector<shogun::SGVector<SGTYPE>>), "header")
{
    jobjectArray SWIG_From_dec(std::vector<shogun::SGVector<SGTYPE>>)
    	(JNIEnv* jenv, const std::vector<shogun::SGVector<SGTYPE>>& string_array)
    {
		int32_t num_strings = string_array.size();

		// class for inner array (the invidivual strings)
		jclass cls = JCALL1(FindClass, jenv, CLASSDESC);

		// create outer array of inner array types
		jobjectArray outer = JCALL3(NewObjectArray, jenv, num_strings, cls, NULL);

		for (auto i : range(num_strings)) {
			auto& string = string_array[i];
			auto slen = string.vlen;
			
			##JNITYPE##Array inner = JCALL1(New##JAVATYPE##Array, jenv, slen);

			// convert to java type and pass to inner array
			SGVector<JNITYPE> inner_data(slen);
			std::copy_n(string.vector, slen, inner_data.vector);

			JCALL4(Set##JAVATYPE##ArrayRegion, jenv, inner, 0, slen, inner_data.vector);
			
			// place inner into outer array
			JCALL3(SetObjectArrayElement, jenv, outer, i, inner);
			JCALL1(DeleteLocalRef, jenv, inner);
		}
		return outer;
    }
}

%typemap(javain) std::vector<shogun::SGVector<SGTYPE>> "$javainput"
%typemap(javaout) std::vector<shogun::SGVector<SGTYPE>> {
	return $jnicall;
}

%apply std::vector<shogun::SGVector<SGTYPE>> { const std::vector<shogun::SGVector<SGTYPE>>& };
%val_in_typemap_with_args(std::vector<shogun::SGVector<SGTYPE>>, jenv);
%val_out_typemap_with_args(std::vector<shogun::SGVector<SGTYPE>>, jenv);
%enddef

TYPEMAP_STRINGFEATURES(bool, boolean, Boolean, jboolean, "[Z")
TYPEMAP_STRINGFEATURES(uint8_t, byte, Byte, jbyte, "[S")
TYPEMAP_STRINGFEATURES(int16_t, short, Short, jshort, "[S")
TYPEMAP_STRINGFEATURES(uint16_t, int, Int, jint, "[I")
TYPEMAP_STRINGFEATURES(int32_t, int, Int, jint, "[I")
TYPEMAP_STRINGFEATURES(uint32_t, long, Long, jlong, "[J")
TYPEMAP_STRINGFEATURES(int64_t, int, Int, jint, "[I")
TYPEMAP_STRINGFEATURES(uint64_t, long, Long, jlong, "[J")
TYPEMAP_STRINGFEATURES(long long, long, Long, jlong, "[J")
TYPEMAP_STRINGFEATURES(float32_t, float, Float, jfloat, "[F")
TYPEMAP_STRINGFEATURES(float64_t, double, Double, jdouble, "[D")

#undef TYPEMAP_STRINGFEATURES

/* input/output typemap for std::vector<shogun::SGVector<char>> */
%typemap(jni) std::vector<shogun::SGVector<char>> %{jobjectArray%}
%typemap(jtype) std::vector<shogun::SGVector<char>> %{String []%}
%typemap(jstype) std::vector<shogun::SGVector<char>> %{String []%}

%fragment(SWIG_AsVal_frag(std::vector<shogun::SGVector<char>>), "header")
{
    int SWIG_AsVal_dec(std::vector<shogun::SGVector<char>>)
    	(JNIEnv* jenv, const jobjectArray& input, std::vector<shogun::SGVector<char>>& strings)
    {
		if (!input) {
			SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
			return SWIG_ERROR;
		}

		int32_t size = JCALL1(GetArrayLength, jenv, input);
		strings.reserve(size);

		for (int32_t i = 0; i < size; i++) {
			jstring jstr = (jstring)JCALL2(GetObjectArrayElement, jenv, input, i);

			int32_t len = JCALL1(GetStringUTFLength, jenv, jstr);
			const char *str = (char *)JCALL2(GetStringUTFChars, jenv, jstr, 0);

			strings.emplace_back(len);
			sg_memcpy(strings.back().vector, str, len);

			JCALL2(ReleaseStringUTFChars, jenv, jstr, str);
		}
		return SWIG_OK;
    }
}

%fragment(SWIG_From_frag(std::vector<shogun::SGVector<char>>), "header")
{
    jobjectArray SWIG_From_dec(std::vector<shogun::SGVector<char>>)
    	(JNIEnv* jenv, const std::vector<shogun::SGVector<char>>& str)
    {
		int32_t num = str.size();

		jclass cls = JCALL1(FindClass, jenv, "java/lang/String");
		jobjectArray res = JCALL3(NewObjectArray, jenv, num, cls, NULL);

		for (int32_t i = 0; i < num; i++) {
			// add null terminator
			SGVector<char> tmp(str[i].size() + 1);
			sg_memcpy(tmp.vector, str[i].vector, str[i].size() * sizeof(char));

			jstring jstr = (jstring)JCALL1(NewStringUTF, jenv, tmp.vector);
			JCALL3(SetObjectArrayElement, jenv, res, i, jstr);
			JCALL1(DeleteLocalRef, jenv, jstr);
		}
		return res;
    }
}

%typemap(javain) std::vector<shogun::SGVector<char>> "$javainput"
%typemap(javaout) std::vector<shogun::SGVector<char>> {
	return $jnicall;
}

%apply std::vector<shogun::SGVector<char>> { const std::vector<shogun::SGVector<char>>& };
%val_in_typemap_with_args(std::vector<shogun::SGVector<char>>, jenv);
%val_out_typemap_with_args(std::vector<shogun::SGVector<char>>, jenv);
