#pragma once

#include "colors.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

namespace bench {

// ******************* Math *******************
double mean(const std::vector<double>& vec) {
    double sum = 0.0;
    for (size_t i = 0; i < vec.size(); i++) {
        sum += vec[i];
    }
    return sum / vec.size();
}

double variance(const std::vector<double>& vec, double mean) {
    double sum = 0.0;
    for (size_t i = 0; i < vec.size(); i++) {
        double diff = vec[i] - mean;
        sum += diff * diff;
    }
    return sum / (vec.size());
}

double standardDeviation(const double variance) { return sqrt(variance); }

// ******************* Benchmarking classes *******************
struct BenchmarkResult {
    std::string funcName;
    std::string testName;

    std::vector<double> times;
    size_t numIterations = 0;
    double mean = 0.0;
    double variance = 0.0;
    double standardDeviation = 0.0;
    double coefficientOfVariation = 0.0;
    double totalDuration = 0.0;
};

class Benchmarker {
  public:
    Benchmarker(std::string name, std::string path, int argc, char* argv[])
        : _name(name), _path(path) {
        if (argc >= 2) {
            _launchTime = argv[1];
        }
        if (argc >= 3) {
            _outpath = argv[2];
        }

        std::cout << "\nRunning benchmarking set " << COL_RED_BOLD << name << COL_NONE << " in "
                  << COL_BLUE_BOLD << path << COL_NONE;
    }

    void printBenchmarkResult(const BenchmarkResult& bench, double minMean, double maxMean) {
        hsv baseGreen{93.66, 0.5861, 0.34};
        // Logarithmic scale
        double B = (1.0 - baseGreen.v) / log10(maxMean / minMean);
        double A = 1.0 - B * log10(maxMean);
        double newV = A + B * log10(bench.mean);
        baseGreen.v = newV;

        rgb newGreen = hsv2rgb(baseGreen);
        int r = (int)(newGreen.r * 255);
        int g = (int)(newGreen.g * 255);
        int b = (int)(newGreen.b * 255);

        std::cout << "⏱  " << COL_PINK_BOLD << _name << "." << bench.funcName << "."
                  << bench.testName << COL_NONE << " ▶︎▶︎ " << COL_RGB(r, g, b)
                  << bench.mean << COL_NONE << " ± " << COL_GREEN << bench.coefficientOfVariation
                  << "%" << COL_NONE << " (" << bench.numIterations << " iterations, "
                  << bench.totalDuration << " s)\n";
    }

    template <typename A, typename B>
    void runBenchmark(std::string funcName, std::string testName, A func, B generator) {
        std::cout << "." << std::flush;
        BenchmarkResult bench;
        bench.funcName = funcName;
        bench.testName = testName;

        bench.totalDuration = 0.0;
        bench.numIterations = 0;
        const auto start = std::chrono::high_resolution_clock::now();
        while (true) {
            bench.times.push_back(timeFunctionCall(func, generator).count());
            bench.numIterations++;
            bench.totalDuration += bench.times.back();

            // Stop when 1000 runs happen, OR if it's taking too long, then at least get 50 runs.
            if (bench.numIterations >= 1000 ||
                (bench.totalDuration >= 1.0 && bench.numIterations >= 50)) {
                break;
            }
        }

        bench.mean = mean(bench.times);
        bench.variance = variance(bench.times, bench.mean);
        bench.standardDeviation = standardDeviation(bench.variance);
        bench.coefficientOfVariation = 100.0 * bench.standardDeviation / bench.mean;

        _results.push_back(bench);
    }

    template <typename A, typename B>
    std::chrono::duration<double> timeFunctionCall(A func, B generator) {
        auto args = generator();
        const auto start = std::chrono::high_resolution_clock::now();
        std::apply(func, args);
        const auto end = std::chrono::high_resolution_clock::now();
        return end - start;
    }

    void finish() {
        if (_results.empty()) {
            std::cout << "\nNo benchmarks ran.\n";
            return;
        }
        std::cout << "\n";

        // Sort the results from most to least expensive mean runtime
        std::sort(_results.begin(), _results.end(),
                  [](BenchmarkResult& a, BenchmarkResult& b) { return a.mean > b.mean; });
        const double maxMean = _results.front().mean;
        const double minMean = _results.back().mean;

        // Output results to screen
        for (const auto& result : _results) {
            printBenchmarkResult(result, minMean, maxMean);
        }

        // Output results to disk if a file path was specified
        if (_outpath != "") {
            std::string path = std::getenv("BUILD_WORKSPACE_DIRECTORY");
            path += "/bench/" + _outpath;
            std::ofstream outfile;
            outfile.open(path, std::ofstream::out | std::ofstream::app);
            std::cout << "Appending results to file " << path << "\n";
            for (const auto& result : _results) {
                outfile << _launchTime << "," << _name << "," << result.funcName << ","
                        << result.testName << "," << result.mean << "," << result.variance << ","
                        << result.standardDeviation << "," << result.coefficientOfVariation << "\n";
            }
            outfile.close();
        }
    }

  private:
    std::vector<BenchmarkResult> _results;
    std::string _name;
    std::string _path;
    std::string _outpath;
    std::string _launchTime;
};

// ******************* Macro-defined interface *******************
#define DECLARE_BENCHMARK_SET(name, argc, argv)                                                    \
    bench::Benchmarker name(#name, __FILE__, argc, argv);

#define RUN_BENCH(name, func, testname, generator)                                                 \
    name.runBenchmark(#func, testname, func, []() generator);

#define END_BENCHMARK_SET(name) name.finish();

} // namespace bench