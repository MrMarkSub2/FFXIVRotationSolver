#pragma once

#include <vector>
#include <algorithm>
#include <map>
#include "utils.h"

// Coordinate-wise storage

template <class T>
class SparseMatrix_t {
public:
	SparseMatrix_t(int rows, int cols)
		: m_rows(rows), m_cols(cols)
	{ 
		if (m_rows < 1 || m_cols < 1)
			throw std::runtime_error("non-real matrix, very cute");
	}

	void addElem(const T& val, int row, int col);
	SparseMatrix_t<T> add(const SparseMatrix_t<T>& m) const;
	std::vector<T> mult(const std::vector<T>& v) const;
	SparseMatrix_t<T> mult(const SparseMatrix_t<T>& m) const;
	// if A is connected to B and B is connected to C, show a direct, weighted connection from A to C
	SparseMatrix_t<T> fullyExpand() const;
	size_t size() const { return m_elems.size(); }
	bool empty() const { return m_elems.empty(); }
	bool isEqual(const SparseMatrix_t<T>& m) const;

private:
	struct Elem {
		Elem() { }
		Elem(const T& v, int r, int c)
			: val(v), row(r), col(c)
		{ }

		class sortByRow {
		public:
			bool operator()(const Elem& a, const Elem& b) {
				return a.row < b.row;
			}
		};

		class sortByCol {
		public:
			bool operator()(const Elem& a, const Elem& b) {
				return a.col < b.col;
			}
		};

		class sortByRowThenCol {
		public:
			bool operator()(const Elem& a, const Elem& b) {
				if (a.row < b.row)
					return true;
				else if (b.row < a.row)
					return false;
				else
					return a.col < b.col;
			}
		};

		T val;
		int row;
		int col;
	};

	mutable std::vector<Elem> m_elems;
	int m_rows;
	int m_cols;
};

template <class T>
inline void SparseMatrix_t<T>::addElem(const T& val, int row, int col)
{
	if (row >= m_rows || col >= m_cols)
		throw std::runtime_error("SparseMatrix out-of-bounds error!");

	// note: no checks for adding multiple values at the same coords
	m_elems.emplace_back(val, row, col);
}

template<class T>
inline SparseMatrix_t<T> SparseMatrix_t<T>::add(const SparseMatrix_t<T>& m) const
{
	if (m_rows != m.m_rows || m_cols != m.m_cols)
		throw std::runtime_error("SparseMatrix add size mismatch");

	std::sort(m_elems.begin(), m_elems.end(), Elem::sortByRowThenCol());
	std::sort(m.m_elems.begin(), m.m_elems.end(), Elem::sortByRowThenCol());

	std::vector<Elem>::const_iterator a_it = m_elems.begin();
	std::vector<Elem>::const_iterator b_it = m.m_elems.begin();

	SparseMatrix_t<T> rval(m_rows, m_cols);

	while (a_it != m_elems.end() && b_it != m.m_elems.end()) {
		bool a_less_b = Elem::sortByRowThenCol()(*a_it, *b_it);
		bool b_less_a = Elem::sortByRowThenCol()(*b_it, *a_it);

		if (!a_less_b && !b_less_a) { //equality
			rval.addElem(a_it->val + b_it->val, a_it->row, a_it->col);
			++a_it;
			++b_it;
		}
		else if (a_less_b) {
			rval.addElem(a_it->val, a_it->row, a_it->col);
			++a_it;
		}
		else if (b_less_a) {
			rval.addElem(b_it->val, b_it->row, b_it->col);
			++b_it;
		}
		else {
			throw std::runtime_error("SparseMatrix addition, elements simultaneously less than!");
		}
	}

	for (; a_it != m_elems.end(); ++a_it) {
		rval.addElem(a_it->val, a_it->row, a_it->col);
	}

	for (; b_it != m.m_elems.end(); ++b_it) {
		rval.addElem(b_it->val, b_it->row, b_it->col);
	}

	return rval;
}

template <class T>
inline std::vector<T> SparseMatrix_t<T>::mult(const std::vector<T>& v) const
{
	if (m_cols != v.size())
		throw std::runtime_error("SparseMatrix matrix cols mismatch vector size");

	std::vector<T> rval(v.size(), 0);

	for (std::vector<Elem>::const_iterator it = m_elems.begin(); it != m_elems.end(); ++it)
		rval[it->row] += it->val * v[it->col];

	return rval;
}

template<class T>
inline SparseMatrix_t<T> SparseMatrix_t<T>::mult(const SparseMatrix_t<T>& m) const
{
	if (m_cols != m.m_rows)
		throw std::runtime_error("SparseMatrix matrix cols mismatch matrix rows");

	// we "convert" the first matrix to CSC and "convert" the second to CSR. This allows us to easily multiply one row by one column
	std::sort(m_elems.begin(), m_elems.end(), Elem::sortByCol());
	std::vector<int> col_ptr(m_cols + 1, 0);
	int col_idx = 0;
	for (int i = 0; i < m_elems.size(); ++i) {
		while (col_idx <= m_elems[i].col)
			col_ptr[col_idx++] = i;
	}
	for (; col_idx <= m_cols; ++col_idx)
		col_ptr[col_idx] = (int)m_elems.size();

	std::sort(m.m_elems.begin(), m.m_elems.end(), Elem::sortByRow());
	std::vector<int> row_ptr(m.m_rows + 1, 0);
	int row_idx = 0;
	for (int i = 0; i < m.m_elems.size(); ++i) {
		while (row_idx <= m.m_elems[i].row)
			row_ptr[row_idx++] = i;
	}
	for (; row_idx <= m.m_rows; ++row_idx)
		row_ptr[row_idx] = (int)m.m_elems.size();

	SparseMatrix_t<T> rval(m_rows, m.m_cols);
	// multiply via outer-product algorithm. there might be faster algorithms, like Strassen's
	std::map<std::pair<int, int>, T> rval_map;
	for (int k = 0; k < m_cols; ++k) {
		SparseMatrix_t<T> k_slice(m_rows, m.m_cols);
		for (col_idx = col_ptr[k]; col_idx < col_ptr[k + 1]; ++col_idx) {
			for (row_idx = row_ptr[k]; row_idx < row_ptr[k + 1]; ++row_idx) {
				k_slice.addElem(m_elems[col_idx].val * m.m_elems[row_idx].val, m_elems[col_idx].row, m.m_elems[row_idx].col);
			}
		}
		rval = rval.add(k_slice);
	}

	return rval;
}

template<class T>
inline SparseMatrix_t<T> SparseMatrix_t<T>::fullyExpand() const
{
	if (m_rows != m_cols)
		return *this;

	// this relies on the assumption that I don't need to put the matrix through some sort of normalization/sigmoid between each step
	SparseMatrix_t<T> rval = *this;
	SparseMatrix_t<T> product = *this;
	for (int i = 0; i < 100; ++i) { // don't get stuck in infinite loop
		product = product.mult(*this); // == (*this) ^ (i + 2)
		if (product.empty()) // no more expansion
			return rval;

		rval = rval.add(product);
	}

	// didn't converge but we oughta be close (enough)
	return rval;
}

namespace {
	template<class T>
	inline bool isEqual(const T& a, const T& b) {
		return a == b;
	}

	template <>
	inline bool isEqual<double>(const double& a, const double& b) {
		return NEAT::IsEssentiallyEqual(a, b);
	}
}

template<class T>
inline bool SparseMatrix_t<T>::isEqual(const SparseMatrix_t<T>& m) const
{
	if (m_rows != m.m_rows || m_cols != m.m_cols)
		throw std::runtime_error("SparseMatrix equality size mismatch");

	std::sort(m_elems.begin(), m_elems.end(), Elem::sortByRowThenCol());
	std::sort(m.m_elems.begin(), m.m_elems.end(), Elem::sortByRowThenCol());

	std::vector<Elem>::const_iterator a_it = m_elems.begin();
	std::vector<Elem>::const_iterator b_it = m.m_elems.begin();

	while (a_it != m_elems.end() && b_it != m.m_elems.end()) {
		bool a_less_b = Elem::sortByRowThenCol()(*a_it, *b_it);
		bool b_less_a = Elem::sortByRowThenCol()(*b_it, *a_it);

		if (a_less_b || b_less_a)
			return false;

		if (!::isEqual(a_it->val, b_it->val))
			return false;

		++a_it;
		++b_it;
	}

	if (a_it != m_elems.end() || b_it != m.m_elems.end())
		return false;

	return true;
}

// this will also get used a ton
template <class T>
inline std::vector<T> addVectors(const std::vector<T>& v1, const std::vector<T>& v2) {
	if (v1.size() != v2.size())
		throw std::runtime_error("vector addition size mismatch");

	std::vector<T> rval(v1.size());
	for (int i = 0; i < v1.size(); ++i)
		rval[i] = v1[i] + v2[i];

	return rval;
}