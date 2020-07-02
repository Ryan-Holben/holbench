#include <bench/benc.h>

#include <iostream>
#include <random>

// Example struct that would normally be defined elsewhere in your code
struct MyClass {
    MyClass() {}

    std::string helloWorld(const std::string& name) { return "Hello, " + name + "!"; }
};

// Lightweight wrapper called by the benchmarker which calls the member function
void TestHelloWorld(MyClass& obj) { obj.helloWorld("Ryan"); }

// A non-member function to test
double SometimesSquare(const double value, const double offset) {
    return value > 0 ? value * value + offset : value;
}

int main(int argc, char* argv[]) {
    DECLARE_BENCHMARK_SET(myBenches, argc, argv);

    // Standard form for a very simple benchmark.  The lambda passed in does any kind of
    // initialization needed before we start measuring runtime.  Its return value is the argument
    // list that will be passed into the function that is benchmarked, and should always be in the
    // form of a std::tuple.  This lambda is run each time before the benchmarked function is
    // called.
    RUN_BENCH(myBenches, TestHelloWorld, "sayHiToRyan", {
        MyClass myObj; // If this instantiation is expensive, including it in the benchmarking would
                       // throw off our measurements.
        return std::make_tuple(myObj);
    });

    // Here's an example where we generate random inputs for our function. It's up to you how to
    // define those inputs, if you want to re-seed the RNG at each call, etc.
    RUN_BENCH(myBenches, SometimesSquare, "randomInputs", {
        thread_local std::random_device randomDevice;
        std::mt19937 rng(randomDevice());
        std::uniform_real_distribution dist(-10.0, 10.0);
        const double value = dist(rng);
        const double offset = 5.1;

        return std::make_tuple(value, offset);
    });

    END_BENCHMARK_SET(myBenches);

    return 0;
}