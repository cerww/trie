#pragma once
#include <algorithm>
#include <vector>
#include <optional>
#include <boost/container/deque.hpp>
#include <memory>

template<typename T>
struct trie {
	trie() :
		m_nodes(1) {}	

	template<typename rng>
	T& operator[](rng&& char_sequence) {
		auto idx = node_at(char_sequence.begin(), char_sequence.end());
		if(!m_nodes[idx].obj) {
			m_nodes_with_stuff.push_back(idx);
			return m_nodes[idx].obj.emplace();
		}
		return *m_nodes[idx].obj;
	}

	T& operator[](const char* char_sequence) {//other template includes the null char ;-;
		//[[assert:char_sequence!=nullptr]];
		return (*this)[std::string_view(char_sequence)];
	}

	void reserve(const size_t n) {
		m_nodes.reserve(n*2);
		m_nodes_with_stuff.reserve(n);
	}

	template<typename Ty>
	struct templated_iterator{
		templated_iterator& operator++(int) noexcept {
			++it;
			return *this;
		}

		templated_iterator operator++() noexcept {
			auto t = *this;
			++it;
			return t;
		}

		bool operator==(templated_iterator other) const noexcept{
			return it == other.it;
		}

		std::vector<size_t>::iterator it;

	};

	struct trie_node{
		trie_node() = default;
		explicit trie_node(int a,trie* b):m_idx(a),m_owner(b){}
		
		std::optional<trie_node> try_advance_with(char c) {
			const auto it = std::find_if(m_owner->m_nodes[m_idx].next.begin(), m_owner->m_nodes[m_idx].next.end(),[&](size_t idx_next) {
				return m_owner->m_nodes[idx_next].c == c;
			});
			if(it == m_owner->m_nodes[m_idx].next.end()) {
				return std::nullopt;
			}
			return trie_node(*it, m_owner);
		}

		bool advance_or_reset(char c) {
			auto t = try_advance_with(c);
			if(!t) {
				m_idx = 0;
				return false;
			}else {
				m_idx = t->m_idx;
				return true;
			}
		}

		T& value() {
			return *m_owner->m_nodes[m_idx].obj;
		}
		bool has_value() const{
			return m_owner->m_nodes[m_idx].obj.has_value();
		}
		bool has_no_children()const {
			return m_owner->m_nodes[m_idx].next.empty();
		}
	private:
		int m_idx = 0;
		trie* m_owner = nullptr;
	};

	trie_node head_node() {
		return trie_node(0, this);
	}
private:
	struct trie_node_internal {
		trie_node_internal() = default;

		explicit trie_node_internal(const char a) : c(a) {}

		char c = 0;
		std::optional<T> obj;
		std::vector<size_t> next = {};
	};
	template<typename it, typename sent>
	std::pair<size_t, it> prefix_node(it start, sent end) {
		size_t current_idx = 0;
		for (; start != end; ++start) {
			const char c = *start;
			const auto iter = std::find_if(m_nodes[current_idx].next.begin(), m_nodes[current_idx].next.end(), [&](size_t a) {
				return m_nodes[a].c == c;
			});

			if (iter == m_nodes[current_idx].next.end())
				return { current_idx,std::forward<it>(start) };

			current_idx = *iter;
		}

		return { current_idx,std::forward<it>(start) };
	}

	template<typename it, typename sent>
	size_t node_at(it&& start, sent&& end) {
		auto[idx, it] = prefix_node(start, end);
		return emplace_range(idx,it, end);
	}

	template<typename it, typename sent>
	std::optional<size_t> try_node_at(it&& start, sent&& end) {
		const auto[idx, ita] = prefix_node(start, end);
		if (ita != end) {
			return std::nullopt;
		}
		return idx;
	}

	template<typename it,typename sent>
	size_t emplace_range(size_t idx,it&& s,sent&& e) {
		for (; s != e; ++s) {
			m_nodes[idx].next.push_back(m_nodes.size());
			idx = m_nodes.size();
			m_nodes.emplace_back(*s);
		}
		return idx;
	}
	friend struct trie_node;
	std::vector<trie_node_internal> m_nodes;
	std::vector<size_t> m_nodes_with_stuff;
};

template<typename T>
struct trie2{
	struct trie_node{
		trie_node() = default;
		explicit trie_node(const char a) : c(a) {}

		char c = 0;
		std::optional<T> obj;
		std::vector<std::unique_ptr<trie_node>> next = {};
	};

	template<typename rng>
	T& operator[](rng&& char_sequence) {
		auto node = node_at(char_sequence.begin(), char_sequence.end());
		if (!node->obj) {
			return node->obj.emplace();
		}
		return *node->obj;
	}
	void reserve(size_t) {//;-;
		
	}
private:
	//returns node and input iterator that it stopped at 
	template<typename it, typename sent>
	std::pair<trie_node*, it> prefix_node(it&& start, sent&& end) {
		trie_node* node = m_root.get();
		for (; start != end; ++start) {
			const char c = *start;
			auto iter = std::find_if(node->next.begin(), node->next.end(), [&](const auto& thing){
				return thing->c == c;
			});

			if (iter == node->next.end())
				return { node,start };

			node = iter->get();
		}

		return { node,start };
	}

	template<typename it, typename sent>
	trie_node* node_at(it&& start, sent&& end) {
		auto[node, it] = prefix_node(start, end);
		return emplace_range(node, it, end);
	}

	template<typename it, typename sent>
	std::optional<trie_node*> try_node_at(it&& start, sent&& end) {
		const auto[node, ita] = prefix_node(start, end);
		if (ita != end) {
			return std::nullopt;
		}
		return node;
	}

	template<typename it, typename sent>
	trie_node* emplace_range(trie_node* node, it&& s, sent&& e) {
		for (; s != e; ++s) {
			node = node->next.emplace_back(std::make_unique<trie_node>(*s)).get();
		}
		return node;
	}
	std::unique_ptr<trie_node> m_root = std::make_unique<trie_node>();
};

template<typename T>
struct trie3 {
	trie3() :
		m_nodes(1) {}


	struct trie_node {
		trie_node() = default;

		explicit trie_node(const char a) : c(a) {}

		char c = 0;
		std::optional<T> obj;
		std::vector<typename boost::container::deque<trie_node>::iterator> next = {};
	};
	using node_iterator = typename boost::container::deque<trie_node>::iterator;

	template<typename rng>
	T& operator[](rng&& char_sequence) {
		auto idx = node_at(char_sequence.begin(), char_sequence.end());
		if (!idx->obj) {
			m_nodes_with_stuff.push_back(idx);
			return idx->obj.emplace();
		}
		return *(idx->obj);
	}

	T& operator[](const char* char_sequence) {//other template includes the null char ;-;
		//[[assert:char_sequence!=nullptr]];
		return (*this)[std::string_view(char_sequence)];
	}

	void reserve(const size_t n) {
		//m_nodes.reserve(n*2);
		m_nodes_with_stuff.reserve(n);
	}

	template<typename Ty>
	struct templated_iterator {
		templated_iterator& operator++(int) noexcept {
			++it;
			return *this;
		}

		templated_iterator operator++() noexcept {
			auto t = *this;
			++it;
			return t;
		}

		bool operator==(templated_iterator other) const noexcept {
			return it == other.it;
		}

		std::vector<size_t>::iterator it;

	};

private:

	template<typename it, typename sent>
	std::pair<node_iterator, it> prefix_node(it&& start, sent&& end) {
		node_iterator current_it = m_nodes.begin();
		for (; start != end; ++start) {
			char c = *start;
			const auto iter = std::find_if(current_it->next.begin(), current_it->next.end(), [&](node_iterator a) {
				return a->c == c;
			});

			if (iter == current_it->next.end())
				return { current_it,std::forward<it>(start) };

			current_it = *iter;
		}

		return { current_it,std::forward<it>(start) };
	}

	template<typename it, typename sent>
	node_iterator node_at(it&& start, sent&& end) {
		auto[idx, it] = prefix_node(start, end);
		return emplace_range(idx, it, end);
	}

	template<typename it, typename sent>
	std::optional<node_iterator> try_node_at(it&& start, sent&& end) {
		const auto[idx, ita] = prefix_node(start, end);
		if (ita != end) {
			return std::nullopt;
		}
		return idx;
	}

	template<typename it, typename sent>
	node_iterator emplace_range(node_iterator idx, it&& s, sent&& e) {
		for (; s != e; ++s) {
			m_nodes.emplace_back(*s);
			auto temp_it = m_nodes.end() - 1;
			idx->next.push_back(temp_it);
			idx = temp_it;
		}
		return idx;
	}

	boost::container::deque<trie_node> m_nodes;
	std::vector<node_iterator> m_nodes_with_stuff;
};


inline size_t to_idx(char c) {
	static constexpr size_t table[] = {
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
		255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50,51, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
	return table[c];
}

template<typename T>
struct trie4 {
	struct trie_node {
		trie_node() = default;

		std::optional<T> obj;
		std::array<std::unique_ptr<trie_node>, 52> next = {};
	};

	template<typename rng>
	T& operator[](rng&& char_sequence) {
		auto node = node_at(char_sequence.begin(), char_sequence.end());
		if (!node->obj) {
			return node->obj.emplace();
		}
		return *node->obj;
	}
	void reserve(size_t) {//;-;

	}
private:
	//returns node and iterator it stopped at
	template<typename it, typename sent>
	std::pair<trie_node*, it> prefix_node(it&& start, sent&& end) {
		trie_node* node = m_root.get();
		for (; start != end; ++start) {
			char c = *start;
			auto& ptr = node->next[to_idx(c)];

			if (!ptr)
				return { node,start };

			node = ptr.get();
		}

		return { node,start };
	}

	template<typename it, typename sent>
	trie_node* node_at(it&& start, sent&& end) {
		auto[node, it] = prefix_node(start, end);
		return emplace_range(node, it, end);
	}

	template<typename it, typename sent>
	std::optional<trie_node*> try_node_at(it&& start, sent&& end) {
		const auto[node, ita] = prefix_node(start, end);
		if (ita != end) {
			return std::nullopt;
		}
		return node;
	}

	template<typename it, typename sent>
	trie_node* emplace_range(trie_node* node, it&& s, sent&& e) {
		for (; s != e; ++s) {
			node = (node->next[to_idx(*s)] = std::make_unique<trie_node>()).get();
		}
		return node;
	}
	std::unique_ptr<trie_node> m_root = std::make_unique<trie_node>();
};