/*
 * Copyright (c) 2016, Shogun-Toolbox e.V. <shogun-team@shogun-toolbox.org>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: 2016 Pan Deng, Soumyajit De, Heiko Strathmann, Viktor Gal
 */

#ifndef EIGEN_BASICOPS_H
#define EIGEN_BASICOPS_H

template <typename T>
void LinalgBackendEigen::add(
    const SGVector<T>& a, const SGVector<T>& b, T alpha, T beta,
    SGVector<T>& result, derived_tag) const
{
	typename SGVector<T>::EigenVectorXtMap a_eig = a;
	typename SGVector<T>::EigenVectorXtMap b_eig = b;
	typename SGVector<T>::EigenVectorXtMap result_eig = result;

	result_eig = alpha * a_eig + beta * b_eig;
}

template <typename T>
void LinalgBackendEigen::add(
    const SGMatrix<T>& a, const SGMatrix<T>& b, T alpha, T beta,
    SGMatrix<T>& result, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap a_eig = a;
	typename SGMatrix<T>::EigenMatrixXtMap b_eig = b;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;

	result_eig = alpha * a_eig + beta * b_eig;
}

template <typename T>
void LinalgBackendEigen::add_col_vec(
    const SGMatrix<T>& A, index_t i, const SGVector<T>& b, SGMatrix<T>& result,
    T alpha, T beta, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap A_eig = A;
	typename SGVector<T>::EigenVectorXtMap b_eig = b;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;

	result_eig.col(i) = alpha * A_eig.col(i) + beta * b_eig;
}

template <typename T>
void LinalgBackendEigen::add_diag(
    SGMatrix<T>& A, const SGVector<T>& b, T alpha, T beta, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap A_eig = A;
	typename SGVector<T>::EigenVectorXtMap b_eig = b;

	A_eig.diagonal() = alpha * A_eig.diagonal() + beta * b_eig;
}

template <typename T>
void LinalgBackendEigen::add_ridge(SGMatrix<T>& A, T beta, derived_tag) const
{
	for (auto i : range(std::min(A.num_rows, A.num_cols)))
		A(i, i) += beta;
}

template <typename T>
void LinalgBackendEigen::pinvh(
    const SGMatrix<T>& A, SGMatrix<T>& result, derived_tag) const
{
	auto m = A.num_rows;
	SGVector<T> s(m);
	SGMatrix<T> V(m, m);
	typename SGMatrix<T>::EigenMatrixXtMap A_eig = A;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;

	eigen_solver_symmetric<T>(A, s, V, m);

	typename SGVector<T>::EigenVectorXtMap s_eig = s;
	typename SGMatrix<T>::EigenMatrixXtMap V_eig = V;

	float64_t pinv_tol = CMath::MACHINE_EPSILON * m * s_eig.real().maxCoeff();
	s_eig.array() = (s_eig.real().array() > pinv_tol)
	                    .select(s_eig.real().array().inverse(), 0);
	result_eig = V_eig * s_eig.asDiagonal() * V_eig.transpose();
}

template <typename T>
void LinalgBackendEigen::pinv(
    const SGMatrix<T>& A, SGMatrix<T>& result, derived_tag) const
{
	auto m = std::min(A.num_cols, A.num_rows);
	typename SGMatrix<T>::EigenMatrixXtMap A_eig = A;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;

	auto svd_eig = A_eig.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV);
	auto s_eig = svd_eig.singularValues().real();
	auto U_eig = svd_eig.matrixU().real();
	auto V_eig = svd_eig.matrixV().real();

	float64_t pinv_tol = CMath::MACHINE_EPSILON * m * s_eig.maxCoeff();
	s_eig.array() =
	    (s_eig.array() > pinv_tol).select(s_eig.array().inverse(), 0);
	result_eig = V_eig * s_eig.asDiagonal() * U_eig.transpose();
}

template <typename T>
void LinalgBackendEigen::rank_update(
    SGMatrix<T>& A, const SGVector<T>& b, T alpha, derived_tag) const
{
	T x;
	T update;
	for (auto j : range(A.num_cols))
	{
		x = b[j];
		for (auto i : range(j))
		{
			update = alpha * x * b[i];
			A(i, j) += update;
			A(j, i) += update;
		}
		A(j, j) += alpha * x * x;
	}
}

template <typename T>
void LinalgBackendEigen::add_col_vec(
    const SGMatrix<T>& A, index_t i, const SGVector<T>& b, SGVector<T>& result,
    T alpha, T beta, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap A_eig = A;
	typename SGVector<T>::EigenVectorXtMap b_eig = b;
	typename SGVector<T>::EigenVectorXtMap result_eig = result;

	result_eig = alpha * A_eig.col(i) + beta * b_eig;
}

template <typename T>
void LinalgBackendEigen::add_vector(
    const SGMatrix<T>& A, const SGVector<T>& b, SGMatrix<T>& result, T alpha,
    T beta, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap A_eig = A;
	typename SGVector<T>::EigenVectorXtMap b_eig = b;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;

	result_eig = (alpha * A_eig).colwise() + beta * b_eig;
}

template <typename T>
void LinalgBackendEigen::add_scalar(SGVector<T>& a, T b, derived_tag) const
{
	typename SGVector<T>::EigenVectorXtMap a_eig = a;
	a_eig = a_eig.array() + b;
}

template <typename T>
void LinalgBackendEigen::add_scalar(SGMatrix<T>& a, T b, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap a_eig = a;
	a_eig = a_eig.array() + b;
}

template <typename T>
T LinalgBackendEigen::dot(
    const SGVector<T>& a, const SGVector<T>& b, derived_tag) const
{
	return (typename SGVector<T>::EigenVectorXtMap(a))
	    .dot(typename SGVector<T>::EigenVectorXtMap(b));
}

/* Helper method to compute elementwise product with Eigen */
template <typename MatrixType>
void element_prod_eigen(
    const MatrixType& A, const MatrixType& B,
    typename SGMatrix<typename MatrixType::Scalar>::EigenMatrixXtMap& result,
    bool transpose_A, bool transpose_B)
{
	if (transpose_A && transpose_B)
		result = A.transpose().array() * B.transpose().array();
	else if (transpose_A)
		result = A.transpose().array() * B.array();
	else if (transpose_B)
		result = A.array() * B.transpose().array();
	else
		result = A.array() * B.array();
}

template <typename T>
void LinalgBackendEigen::element_prod(
    const SGMatrix<T>& a, const SGMatrix<T>& b, SGMatrix<T>& result,
    bool transpose_A, bool transpose_B, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap a_eig = a;
	typename SGMatrix<T>::EigenMatrixXtMap b_eig = b;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;

	element_prod_eigen(a_eig, b_eig, result_eig, transpose_A, transpose_B);
}

template <typename T>
void LinalgBackendEigen::element_prod(
    const linalg::Block<SGMatrix<T>>& a, const linalg::Block<SGMatrix<T>>& b,
    SGMatrix<T>& result, bool transpose_A, bool transpose_B, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap a_eig = a.m_matrix;
	typename SGMatrix<T>::EigenMatrixXtMap b_eig = b.m_matrix;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;

	Eigen::Block<typename SGMatrix<T>::EigenMatrixXtMap> a_block =
	    a_eig.block(a.m_row_begin, a.m_col_begin, a.m_row_size, a.m_col_size);
	Eigen::Block<typename SGMatrix<T>::EigenMatrixXtMap> b_block =
	    b_eig.block(b.m_row_begin, b.m_col_begin, b.m_row_size, b.m_col_size);

	element_prod_eigen(a_block, b_block, result_eig, transpose_A, transpose_B);
}

template <typename T>
void LinalgBackendEigen::element_prod(
    const SGVector<T>& a, const SGVector<T>& b, SGVector<T>& result,
    derived_tag) const
{
	typename SGVector<T>::EigenVectorXtMap a_eig = a;
	typename SGVector<T>::EigenVectorXtMap b_eig = b;
	typename SGVector<T>::EigenVectorXtMap result_eig = result;

	result_eig = a_eig.array() * b_eig.array();
}

template <typename T>
void LinalgBackendEigen::exponent(
    const SGVector<T>& a, SGVector<T>& result, derived_tag) const
{
	typename SGVector<T>::EigenVectorXtMap a_eig = a;
	typename SGVector<T>::EigenVectorXtMap result_eig = result;
	result_eig = a_eig.array().exp();
}

template <typename T>
void LinalgBackendEigen::exponent(
    const SGMatrix<T>& a, SGMatrix<T>& result, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap a_eig = a;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;
	result_eig = a_eig.array().exp();
}

template <typename T>
void LinalgBackendEigen::matrix_prod(
    const SGMatrix<T>& a, const SGVector<T>& b, SGVector<T>& result,
    bool transpose, bool transpose_B, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap a_eig = a;
	typename SGVector<T>::EigenVectorXtMap b_eig = b;
	typename SGVector<T>::EigenVectorXtMap result_eig = result;

	if (transpose)
		result_eig = a_eig.transpose() * b_eig;
	else
		result_eig = a_eig * b_eig;
}

template <typename T>
void LinalgBackendEigen::matrix_prod(
    const SGMatrix<T>& a, const SGMatrix<T>& b, SGMatrix<T>& result,
    bool transpose_A, bool transpose_B, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap a_eig = a;
	typename SGMatrix<T>::EigenMatrixXtMap b_eig = b;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;

	if (transpose_A && transpose_B)
		result_eig = a_eig.transpose() * b_eig.transpose();

	else if (transpose_A)
		result_eig = a_eig.transpose() * b_eig;

	else if (transpose_B)
		result_eig = a_eig * b_eig.transpose();

	else
		result_eig = a_eig * b_eig;
}

template <typename T>
void LinalgBackendEigen::scale(
    const SGVector<T>& a, T alpha, SGVector<T>& result, derived_tag) const
{
	typename SGVector<T>::EigenVectorXtMap a_eig = a;
	typename SGVector<T>::EigenVectorXtMap result_eig = result;

	result_eig = alpha * a_eig;
}

template <typename T>
void LinalgBackendEigen::scale(
    const SGMatrix<T>& a, T alpha, SGMatrix<T>& result, derived_tag) const
{
	typename SGMatrix<T>::EigenMatrixXtMap a_eig = a;
	typename SGMatrix<T>::EigenMatrixXtMap result_eig = result;

	result_eig = alpha * a_eig;
}

#endif