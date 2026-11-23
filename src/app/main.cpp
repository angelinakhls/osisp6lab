#include "dll_loader.h"
#include "logger.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

struct AppConfig {
    std::string dllPath = "KhlusMathLib.dll";
    std::string mode = "all"; // demo | benchmark | test-errors | all
    std::string exportCsv = "data/benchmark_results.csv";
    std::string lifecycleCsv = "data/lifecycle_results.csv";
    std::string logFile = "lab06_run.log";
    unsigned int studentId = 114;
    unsigned int seed = 1000 + 114 * 97; // 12058
    int runs = 3;
    int iterations = 1000;
    std::vector<std::size_t> sizes = {100, 1000, 10000};
};

static void PrintUsage() {
    std::cout << "Lab 06 - Variant 28: Mathematical DLL with Explicit Loading\n"
              << "Usage:\n"
              << "  lab06_app.exe [options]\n\n"
              << "Options:\n"
              << "  --dll <path>         Path to DLL (default: KhlusMathLib.dll)\n"
              << "  --mode <mode>        demo | benchmark | test-errors | all (default: all)\n"
              << "  --size <N>           Single array size for demo/benchmark\n"
              << "  --student-id <id>    Student ID (default: 114 -> SEED=12058)\n"
              << "  --seed <S>           Explicit PRNG seed\n"
              << "  --runs <R>           Number of runs per configuration (default: 3)\n"
              << "  --iterations <K>     Repetitions per measurement for QPC stability (default: 1000)\n"
              << "  --export <path>      CSV path for benchmark results (default: data/benchmark_results.csv)\n"
              << "  --log <path>         Log file path (default: lab06_run.log)\n"
              << "  --help               Show this message\n";
}

static bool ParseArgs(int argc, char* argv[], AppConfig& cfg) {
    bool customSeed = false;
    bool customSizes = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto nextVal = [&](const std::string& name) -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "[ERROR] Missing value for option: " << name << "\n";
                return nullptr;
            }
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            return false;
        } else if (arg == "--dll") {
            const char* v = nextVal(arg);
            if (!v) return false;
            cfg.dllPath = v;
        } else if (arg == "--mode") {
            const char* v = nextVal(arg);
            if (!v) return false;
            cfg.mode = v;
        } else if (arg == "--size") {
            const char* v = nextVal(arg);
            if (!v) return false;
            long long n = std::atoll(v);
            if (n <= 0) {
                std::cerr << "[ERROR] Invalid --size value: " << v << "\n";
                return false;
            }
            if (!customSizes) {
                cfg.sizes.clear();
                customSizes = true;
            }
            cfg.sizes.push_back(static_cast<std::size_t>(n));
        } else if (arg == "--student-id") {
            const char* v = nextVal(arg);
            if (!v) return false;
            int id = std::atoi(v);
            if (id <= 0) {
                std::cerr << "[ERROR] Invalid --student-id value: " << v << "\n";
                return false;
            }
            cfg.studentId = static_cast<unsigned int>(id);
            if (!customSeed) {
                cfg.seed = 1000 + cfg.studentId * 97;
            }
        } else if (arg == "--seed") {
            const char* v = nextVal(arg);
            if (!v) return false;
            cfg.seed = static_cast<unsigned int>(std::strtoul(v, nullptr, 10));
            customSeed = true;
        } else if (arg == "--runs") {
            const char* v = nextVal(arg);
            if (!v) return false;
            cfg.runs = std::atoi(v);
            if (cfg.runs < 1) {
                std::cerr << "[ERROR] --runs must be >= 1\n";
                return false;
            }
        } else if (arg == "--iterations") {
            const char* v = nextVal(arg);
            if (!v) return false;
            cfg.iterations = std::atoi(v);
            if (cfg.iterations < 1) {
                std::cerr << "[ERROR] --iterations must be >= 1\n";
                return false;
            }
        } else if (arg == "--export") {
            const char* v = nextVal(arg);
            if (!v) return false;
            cfg.exportCsv = v;
        } else if (arg == "--log") {
            const char* v = nextVal(arg);
            if (!v) return false;
            cfg.logFile = v;
        } else {
            std::cerr << "[ERROR] Unknown argument: " << arg << "\n";
            PrintUsage();
            return false;
        }
    }
    return true;
}

static std::vector<double> GenerateArray(std::size_t count, unsigned int seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-1000.0, 1000.0);
    std::vector<double> arr(count);
    for (std::size_t i = 0; i < count; ++i) {
        arr[i] = dist(rng);
    }
    return arr;
}

static bool RunDemo(const AppConfig& cfg, Logger& logger) {
    std::cout << "\n=== 1. FUNCTIONAL DEMO & VERIFICATION ===\n";
    MathDllLoader loader(logger);

    if (!loader.Load(cfg.dllPath)) {
        return false;
    }
    if (!loader.ResolveAllFunctions()) {
        loader.Unload();
        return false;
    }

    const std::size_t demoSize = cfg.sizes.empty() ? 100 : cfg.sizes.front();
    std::vector<double> data = GenerateArray(demoSize, cfg.seed);

    double sum = 0.0, minVal = 0.0, maxVal = 0.0, avg = 0.0;
    const auto& fn = loader.Functions();

    bool okSum = fn.ArraySum(data.data(), data.size(), &sum);
    bool okMin = fn.ArrayMin(data.data(), data.size(), &minVal);
    bool okMax = fn.ArrayMax(data.data(), data.size(), &maxVal);
    bool okAvg = fn.ArrayAverage(data.data(), data.size(), &avg);

    logger.Log("DLL_CALL", "All4Functions", (okSum && okMin && okMax && okAvg) ? "OK" : "ERROR");

    // Independent reference check in host app
    double refSum = 0.0, refMin = data[0], refMax = data[0];
    for (double x : data) {
        refSum += x;
        if (x < refMin) refMin = x;
        if (x > refMax) refMax = x;
    }
    double refAvg = refSum / static_cast<double>(data.size());

    const double eps = 1e-9;
    bool verified = (std::fabs(sum - refSum) < eps) &&
                    (std::fabs(minVal - refMin) < eps) &&
                    (std::fabs(maxVal - refMax) < eps) &&
                    (std::fabs(avg - refAvg) < eps);

    std::cout << std::fixed << std::setprecision(6)
              << "Array size   : " << demoSize << " (SEED=" << cfg.seed << ")\n"
              << "ArraySum     : " << sum << "\n"
              << "ArrayMin     : " << minVal << "\n"
              << "ArrayMax     : " << maxVal << "\n"
              << "ArrayAverage : " << avg << "\n"
              << "Verification : " << (verified ? "PASSED" : "FAILED") << "\n";

    return loader.Unload() && verified;
}

static void RunErrorTests(const AppConfig& cfg, Logger& logger) {
    std::cout << "\n=== 2. ERROR HANDLING SCENARIOS ===\n";

    // Test 1: Missing DLL file
    std::cout << "\n[Test 1] Loading non-existent DLL...\n";
    {
        MathDllLoader loader(logger);
        bool loaded = loader.Load("NonExistentMathLib_404.dll");
        std::cout << "Result: " << (!loaded ? "PASSED (error handled gracefully)" : "FAILED") << "\n";
    }

    // Test 2: Missing exported function
    std::cout << "\n[Test 2] Resolving non-existent exported function...\n";
    {
        MathDllLoader loader(logger);
        if (loader.Load(cfg.dllPath)) {
            FARPROC badProc = loader.ResolveSymbol("ArrayMedian_DoesNotExist");
            std::cout << "Result: " << (badProc == nullptr ? "PASSED (error handled gracefully)" : "FAILED") << "\n";
            loader.Unload();
        }
    }

    // Test 3: Invalid arguments passed to DLL functions (empty array / nullptr)
    std::cout << "\n[Test 3] Calling DLL functions with invalid arguments (nullptr / count=0)...\n";
    {
        MathDllLoader loader(logger);
        if (loader.Load(cfg.dllPath) && loader.ResolveAllFunctions()) {
            double out = 0.0;
            bool r1 = loader.Functions().ArraySum(nullptr, 10, &out);
            bool r2 = loader.Functions().ArrayAverage(&out, 0, &out);
            bool passed = (!r1 && !r2);
            logger.Log("DLL_INVALID_ARGS", "ArraySum/ArrayAverage", passed ? "OK" : "ERROR", 0,
                       "Rejected invalid pointers/zero size");
            std::cout << "Result: " << (passed ? "PASSED (invalid inputs rejected)" : "FAILED") << "\n";
            loader.Unload();
        }
    }
}

static bool RunBenchmarks(const AppConfig& cfg, Logger& logger) {
    std::cout << "\n=== 3. PERFORMANCE BENCHMARKS (QPC) ===\n";
    std::cout << "SEED=" << cfg.seed << ", Runs=" << cfg.runs
              << ", Iterations per run=" << cfg.iterations << "\n\n";

    // Part A: DLL lifecycle measurements (LoadLibrary / GetProcAddress / FreeLibrary)
    std::ofstream lifeOut(cfg.lifecycleCsv);
    if (lifeOut.is_open()) {
        lifeOut << "run,load_ms,getproc_ms,free_ms,total_ms\n";
    }

    double sumLoad = 0.0, sumProc = 0.0, sumFree = 0.0;
    for (int r = 1; r <= cfg.runs; ++r) {
        MathDllLoader loader(logger);
        double tLoad = 0.0, tProc = 0.0, tFree = 0.0;
        if (!loader.Load(cfg.dllPath, &tLoad)) return false;
        if (!loader.ResolveAllFunctions(&tProc)) return false;
        if (!loader.Unload(&tFree)) return false;

        sumLoad += tLoad;
        sumProc += tProc;
        sumFree += tFree;
        if (lifeOut.is_open()) {
            lifeOut << r << "," << std::fixed << std::setprecision(6)
                    << tLoad << "," << tProc << "," << tFree << "," << (tLoad + tProc + tFree) << "\n";
        }
    }

    std::cout << "\n--- DLL Lifecycle Averages (" << cfg.runs << " runs) ---\n"
              << std::fixed << std::setprecision(4)
              << "LoadLibraryA   : " << (sumLoad / cfg.runs) << " ms\n"
              << "GetProcAddress : " << (sumProc / cfg.runs) << " ms (4 functions)\n"
              << "FreeLibrary    : " << (sumFree / cfg.runs) << " ms\n\n";

    // Part B: Array computation measurements
    MathDllLoader loader(logger);
    if (!loader.Load(cfg.dllPath) || !loader.ResolveAllFunctions()) {
        return false;
    }
    const auto& fn = loader.Functions();

    std::ofstream csvOut(cfg.exportCsv);
    if (csvOut.is_open()) {
        csvOut << "size,run,sum_ms,min_ms,max_ms,avg_ms,total_ms\n";
    }

    std::cout << std::left << std::setw(10) << "Size"
              << std::setw(12) << "Run 1 (ms)"
              << std::setw(12) << "Run 2 (ms)"
              << std::setw(12) << "Run 3 (ms)"
              << std::setw(12) << "Mean (ms)" << "\n"
              << std::string(58, '-') << "\n";

    for (std::size_t n : cfg.sizes) {
        std::vector<double> data = GenerateArray(n, cfg.seed);
        volatile double sink = 0.0; // prevents dead-code elimination

        std::vector<double> runTotals;
        for (int r = 1; r <= cfg.runs; ++r) {
            double res = 0.0;

            double t0 = MathDllLoader::NowMs();
            for (int it = 0; it < cfg.iterations; ++it) {
                fn.ArraySum(data.data(), n, &res);
                sink = res;
            }
            double tSum = MathDllLoader::NowMs() - t0;

            t0 = MathDllLoader::NowMs();
            for (int it = 0; it < cfg.iterations; ++it) {
                fn.ArrayMin(data.data(), n, &res);
                sink = res;
            }
            double tMin = MathDllLoader::NowMs() - t0;

            t0 = MathDllLoader::NowMs();
            for (int it = 0; it < cfg.iterations; ++it) {
                fn.ArrayMax(data.data(), n, &res);
                sink = res;
            }
            double tMax = MathDllLoader::NowMs() - t0;

            t0 = MathDllLoader::NowMs();
            for (int it = 0; it < cfg.iterations; ++it) {
                fn.ArrayAverage(data.data(), n, &res);
                sink = res;
            }
            double tAvg = MathDllLoader::NowMs() - t0;

            double total = tSum + tMin + tMax + tAvg;
            runTotals.push_back(total);

            if (csvOut.is_open()) {
                csvOut << n << "," << r << "," << std::fixed << std::setprecision(6)
                       << tSum << "," << tMin << "," << tMax << "," << tAvg << "," << total << "\n";
            }
        }
        (void)sink;

        double mean = std::accumulate(runTotals.begin(), runTotals.end(), 0.0) / static_cast<double>(runTotals.size());
        std::cout << std::left << std::setw(10) << n << std::fixed << std::setprecision(4);
        for (int i = 0; i < 3; ++i) {
            if (i < static_cast<int>(runTotals.size())) {
                std::cout << std::setw(12) << runTotals[i];
            } else {
                std::cout << std::setw(12) << "-";
            }
        }
        std::cout << std::setw(12) << mean << "\n";
    }

    if (csvOut.is_open()) {
        logger.Log("CSV_EXPORT", cfg.exportCsv, "OK");
    }

    return loader.Unload();
}

int main(int argc, char* argv[]) {
    AppConfig cfg;
    if (!ParseArgs(argc, argv, cfg)) {
        return (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) ? 0 : 1;
    }

    Logger logger(cfg.logFile);
    logger.Log("APP_START", "Lab06_Var28", "OK", 0, "SEED=" + std::to_string(cfg.seed));

    bool ok = true;
    if (cfg.mode == "demo") {
        ok = RunDemo(cfg, logger);
    } else if (cfg.mode == "test-errors") {
        RunErrorTests(cfg, logger);
    } else if (cfg.mode == "benchmark") {
        ok = RunBenchmarks(cfg, logger);
    } else if (cfg.mode == "all") {
        ok = RunDemo(cfg, logger);
        RunErrorTests(cfg, logger);
        ok = RunBenchmarks(cfg, logger) && ok;
    } else {
        std::cerr << "[ERROR] Unknown mode: " << cfg.mode << "\n";
        PrintUsage();
        return 1;
    }

    logger.Log("APP_EXIT", "Lab06_Var28", ok ? "OK" : "ERROR");
    return ok ? 0 : 1;
}
