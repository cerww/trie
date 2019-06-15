#include "trie.h"
#include <random>
#include <array>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <iostream>


std::string random_string(std::mt19937& eng) {
	std::string ret;
	ret.resize(eng() & 63);
	std::uniform_int_distribution<int> dist('A', 'Z' + 0 * 26);
	std::generate(ret.begin(), ret.end(), [&]() {
		auto t = dist(eng);
		return t - (26 * (t > 'Z')) + 32 * (t > 'Z');
		});
	return ret;
}

template<typename trie>
void test() {
	trie treea;

	volatile int count = 90000;
	std::vector<std::string> strs(count);
	std::random_device r;
	std::mt19937 eng(r());

	std::array<std::string, 10> prefixes = {
		"cat",
		"blue",
		"qweqweqwe",
		"potato",
		"uwu",
		"pota",
		"black",
		"uwert",
		"hatmaro"
	};

	std::generate(strs.begin(), strs.end(), [&, idx = 0]() mutable{
		return prefixes[idx++ % 10] + random_string(eng);
	});

	std::sort(strs.begin(), strs.end(), [](const auto& a, const auto& b) {
		return a.size() > b.size();
		});

	treea.reserve(count * 2);

	for (const auto& s : strs) {
		treea[s] = s + "ssss";
	}

	for (const auto& s : strs) {
		treea[s] += "cat";
		treea[s + "c"] = s;
	}
}

template<template<typename> typename trie_t>
void test_trie_speed2() {
	trie_t<int> trie;

	std::random_device r;
	std::mt19937 engine(r());

	std::atomic<int> tries = 10000;//so it's not optimized;
	for (int i = 0; i < tries.load(std::memory_order_relaxed); ++i) {
		auto t = random_string(engine);
		trie[t] = t.size();
	}
}

int main() {
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		test<trie<std::string>>();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	}
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		test<trie2<std::string>>();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	}
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		test<trie3<std::string>>();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	} 
	
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		test<trie4<std::string>>();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	}
	
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		test<std::unordered_map<std::string,std::string>>();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	}
}
