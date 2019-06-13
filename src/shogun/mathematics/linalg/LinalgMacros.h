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

#define DEFINE_FOR_ALL_PTYPE(METHODNAME, Container)                            \
	METHODNAME(bool, Container);                                               \
	METHODNAME(char, Container);                                               \
	METHODNAME(int8_t, Container);                                             \
	METHODNAME(uint8_t, Container);                                            \
	METHODNAME(int16_t, Container);                                            \
	METHODNAME(uint16_t, Container);                                           \
	METHODNAME(int32_t, Container);                                            \
	METHODNAME(uint32_t, Container);                                           \
	METHODNAME(int64_t, Container);                                            \
	METHODNAME(uint64_t, Container);                                           \
	METHODNAME(float32_t, Container);                                          \
	METHODNAME(float64_t, Container);                                          \
	METHODNAME(floatmax_t, Container);                                         \
	METHODNAME(complex128_t, Container);

#define DEFINE_FOR_ALL_PTYPE_EXCEPT_FLOAT64(METHODNAME, Container)             \
	METHODNAME(bool, Container);                                               \
	METHODNAME(char, Container);                                               \
	METHODNAME(int8_t, Container);                                             \
	METHODNAME(uint8_t, Container);                                            \
	METHODNAME(int16_t, Container);                                            \
	METHODNAME(uint16_t, Container);                                           \
	METHODNAME(int32_t, Container);                                            \
	METHODNAME(uint32_t, Container);                                           \
	METHODNAME(int64_t, Container);                                            \
	METHODNAME(uint64_t, Container);                                           \
	METHODNAME(float32_t, Container);                                          \
	METHODNAME(floatmax_t, Container);                                         \
	METHODNAME(complex128_t, Container);

#define DEFINE_FOR_NON_COMPLEX_PTYPE(METHODNAME, Container)                    \
	METHODNAME(bool, Container);                                               \
	METHODNAME(char, Container);                                               \
	METHODNAME(int8_t, Container);                                             \
	METHODNAME(uint8_t, Container);                                            \
	METHODNAME(int16_t, Container);                                            \
	METHODNAME(uint16_t, Container);                                           \
	METHODNAME(int32_t, Container);                                            \
	METHODNAME(uint32_t, Container);                                           \
	METHODNAME(int64_t, Container);                                            \
	METHODNAME(uint64_t, Container);                                           \
	METHODNAME(float32_t, Container);                                          \
	METHODNAME(float64_t, Container);                                          \
	METHODNAME(floatmax_t, Container);

#define DEFINE_FOR_NON_INTEGER_PTYPE(METHODNAME, Container)                    \
	METHODNAME(float32_t, Container);                                          \
	METHODNAME(float64_t, Container);                                          \
	METHODNAME(floatmax_t, Container);                                         \
	METHODNAME(complex128_t, Container);

#define DEFINE_FOR_NON_INTEGER_REAL_PTYPE(METHODNAME, Container)               \
	METHODNAME(float32_t, Container);                                          \
	METHODNAME(float64_t, Container);                                          \
	METHODNAME(floatmax_t, Container);

#define DEFINE_FOR_NUMERIC_PTYPE(METHODNAME, Container)                        \
	METHODNAME(char, Container);                                               \
	METHODNAME(int8_t, Container);                                             \
	METHODNAME(uint8_t, Container);                                            \
	METHODNAME(int16_t, Container);                                            \
	METHODNAME(uint16_t, Container);                                           \
	METHODNAME(int32_t, Container);                                            \
	METHODNAME(uint32_t, Container);                                           \
	METHODNAME(int64_t, Container);                                            \
	METHODNAME(uint64_t, Container);                                           \
	METHODNAME(float32_t, Container);                                          \
	METHODNAME(float64_t, Container);                                          \
	METHODNAME(floatmax_t, Container);
