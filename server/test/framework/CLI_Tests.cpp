#include "gtest/gtest.h"

#include "BaseTest.h"
#include "KleeGenerator.h"
#include "SARIFGenerator.h"
#include "Paths.h"
#include "Server.h"
#include "TestUtils.h"
#include "utils/CLIUtils.h"

#include <protobuf/testgen.grpc.pb.h>

#include <set>
#include <map>

namespace {
    using grpc::Channel;
    using grpc::ClientContext;
    using testUtils::checkTestCasePredicates;
    using testUtils::createArgvVector;
    using testUtils::createLineRequest;

    class CLI_Test : public BaseTest {
    protected:
        CLI_Test() : BaseTest("cli") {
        }

        const std::string resultsDirectoryName = "utbot-report";
        const std::string buildDirectoryName = "build_clang";

        const std::string assertion_failures = "assertion_failures";
        const std::string basic_functions = "basic_functions";
        const std::string complex_structs = "complex_structs";
        const std::string inner_basic_functions = "inner/inner_basic_functions";
        const std::string snippet = "snippet";


        const fs::path assertion_failures_test_cpp =
            getPathToGeneratedTestFileByTestedFile(assertion_failures);
        const fs::path basic_functions_tests_cpp =
            getPathToGeneratedTestFileByTestedFile(basic_functions);
        const fs::path complex_structs_test_cpp =
            getPathToGeneratedTestFileByTestedFile(complex_structs);
        const fs::path inner_basic_functions_test_cpp =
            getPathToGeneratedTestFileByTestedFile(inner_basic_functions);
        const fs::path snippet_test_cpp = getPathToGeneratedTestFileByTestedFile(snippet);

        const fs::path assertion_failures_test_h =
            getPathToGeneratedTestHeaderFileByTestedFile(assertion_failures);
        const fs::path basic_functions_tests_h =
            getPathToGeneratedTestHeaderFileByTestedFile(basic_functions);
        const fs::path complex_structs_test_h =
            getPathToGeneratedTestHeaderFileByTestedFile(complex_structs);
        const fs::path inner_basic_functions_test_h =
            getPathToGeneratedTestHeaderFileByTestedFile(inner_basic_functions);
        const fs::path snippet_test_h = getPathToGeneratedTestHeaderFileByTestedFile(snippet);

        std::vector<fs::path> allProjectTestFiles = {
            assertion_failures_test_cpp,    basic_functions_tests_cpp,   complex_structs_test_cpp,
            inner_basic_functions_test_cpp, assertion_failures_test_h,   basic_functions_tests_h,
            complex_structs_test_h,         inner_basic_functions_test_h
        };

        std::vector<fs::path> allProjectSrcFiles = {
            "assertion_failures.c", "basic_functions.c", "complex_structs.c", "inner/inner_basic_functions.c"
        };

        void SetUp() override {
            clearTestDirectory();
            clearDirectory(suitePath / resultsDirectoryName);
            clearEnv(CompilationUtils::CompilerName::CLANG);
        }

        static void runCommandLine(const std::vector<std::string> &&args) {
            CLI::App app{ "Unit tests auto-generation tool for C projects." };
            std::vector<char *> argv = createArgvVector(args);
            CLIUtils::parse(static_cast<int>(argv.size()) - 1, argv.data(), app);
        }

        void checkTestDirectory(std::vector<fs::path> filePaths) {
            for (const auto &filePath : filePaths) {
                EXPECT_TRUE(fs::exists(filePath)) << testUtils::fileNotExistsMessage(filePath);
            }
            std::set<fs::path> fileSet(filePaths.begin(), filePaths.end());

            FileSystemUtils::RecursiveDirectoryIterator directoryIterator(getTestDirectory());
            for (auto &&it : directoryIterator) {
                if (!it.is_regular_file() ||
                it.path().extension() == Paths::MAKEFILE_EXTENSION ||
                StringUtils::endsWith(Paths::removeExtension(it.path().filename()).string(), Paths::MAKE_WRAPPER_SUFFIX)) {
                    continue;
                }
                EXPECT_TRUE(Paths::isSubPathOf(getStubsDirectory(), it.path()) ||
                            fileSet.count(it.path()))
                    << testUtils::unexpectedFileMessage(it.path());
            }
        }

        void checkResultsDirectory() {
            FileSystemUtils::RecursiveDirectoryIterator directoryIterator(suitePath /
                                                                          resultsDirectoryName);
            EXPECT_EQ(directoryIterator.size(), 3);
            for (auto &&it : directoryIterator) {
                EXPECT_TRUE(it.is_regular_file());
            }
        }
    };

    std::size_t replaceAll(std::string& inout, std::string_view what, std::string_view with)
    {
        std::size_t count{};
        for (std::string::size_type pos{};
             std::string::npos != (pos = inout.find(what.data(), pos, what.length()));
             pos += with.length(), ++count) {
            inout.replace(pos, what.length(), with.data(), with.length());
        }
        return count;
    }

    std::string getNormalizedContent(const fs::path &name) {
        EXPECT_TRUE(fs::exists(name)) << "File " << name.c_str() << " don't exists!";
        std::ifstream src(name, std::ios_base::in);
        std::string content((std::istreambuf_iterator<char>(src)), std::istreambuf_iterator<char>());
        replaceAll(content, "\r\n", "\n");
        return content;
    }

    std::string filterUnstableParams(const std::string &context) {
        std::stringstream ins(context);
        std::stringstream outs;
        std::string line;
        while (getline(ins, line)) {
            if (line.find(R"x("startLine")x") != std::string::npos) {
                outs << "[startLine replacement]" << "\n";
            } else if (line.find(R"x( (test)")x") != std::string::npos) {
                outs << "[test name replacement]" << "\n";
            } else {
                outs << line;
            }
        }
        return outs.str();
    }

    std::map<std::string, std::string> getOrderedRecords(const json &rep) {
        json results = rep["runs"][0]["results"];
        std::map<std::string, std::string> records;
        for (const auto &it : results) {
            std::string problemDescription = it["message"]["text"];

            std::string srcFile = it["locations"][0]["physicalLocation"]["artifactLocation"]["uri"];
            int lineNo = it["locations"][0]["physicalLocation"]["region"]["startLine"];

            std::stringstream key;
            key << problemDescription << "\n"
                << srcFile << ":" << lineNo;

            records.insert(std::pair(key.str(), filterUnstableParams(it.dump(2))));
        }
        return records;
    }

    void compareSARIFFiles(const fs::path &golden, const fs::path &real) {
        json gjs = json::parse(getNormalizedContent(golden));
        json rjs = json::parse(getNormalizedContent(real));

        EXPECT_EQ(getOrderedRecords(gjs), getOrderedRecords(rjs));
    }

    TEST_F(CLI_Test, Generate_Project_Tests) {
        fs::remove(suitePath / sarif::SARIF_DIR_NAME / sarif::SARIF_FILE_NAME);

        runCommandLine({ "./utbot", "generate", "--project-path", suitePath, "--build-dir",
                         buildDirectoryName, "project" });
        checkTestDirectory(allProjectTestFiles);

        compareSARIFFiles(
            suitePath / "goldenImage" / sarif::SARIF_DIR_NAME /sarif::SARIF_FILE_NAME,
            suitePath / sarif::SARIF_DIR_NAME / sarif::SARIF_FILE_NAME);
    }

    TEST_F(CLI_Test, Generate_File_Tests) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath, "--build-dir",
                         buildDirectoryName, "file", "--file-path",
                         suitePath / "basic_functions.c" });
        checkTestDirectory({ basic_functions_tests_cpp, basic_functions_tests_h });
    }

    TEST_F(CLI_Test, Generate_Folder_Tests) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath, "--build-dir",
                         buildDirectoryName, "folder", "--folder-path", suitePath / "inner" });
        checkTestDirectory({ inner_basic_functions_test_cpp, inner_basic_functions_test_h });
    }

    TEST_F(CLI_Test, Generate_Function_Tests) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath, "--build-dir",
                         buildDirectoryName, "function", "--file-path",
                         suitePath / "complex_structs.c", "--line-number", "39" });
        checkTestDirectory({ complex_structs_test_cpp, complex_structs_test_h });
    }

    TEST_F(CLI_Test, Generate_Line_Tests) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath, "--build-dir",
                         buildDirectoryName, "function", "--file-path",
                         suitePath / "basic_functions.c", "--line-number", "24" });
        checkTestDirectory({ basic_functions_tests_cpp, basic_functions_tests_h });
    }

    TEST_F(CLI_Test, Generate_Assertion_Tests) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath, "--build-dir",
                         buildDirectoryName, "assertion", "--file-path",
                         suitePath / "assertion_failures.c", "--line-number", "4" });
        checkTestDirectory({ assertion_failures_test_cpp, assertion_failures_test_h });
    }

    TEST_F(CLI_Test, Generate_Predicate_Tests) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath, "--build-dir",
                         buildDirectoryName, "predicate", "--file-path",
                         suitePath / "basic_functions.c", "--line-number", "5", "--validation-type",
                         "int64", "--predicate", "==", "--return-value", "8" });
        checkTestDirectory({ basic_functions_tests_cpp, basic_functions_tests_h });
    }

    TEST_F(CLI_Test, Generate_Snippet_Tests) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath, "snippet",
                         "--file-path", suitePath / "snippet.c" });
        checkTestDirectory({ snippet_test_cpp, snippet_test_h });
    }

    TEST_F(CLI_Test, Run_All_Tests) {
        clearTestDirectory();
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath,
                         "--build-dir", buildDirectoryName, "project" });
        checkTestDirectory(allProjectTestFiles);
        testUtils::checkGenerationStatsCSV(suitePath / resultsDirectoryName / "generation-stats.csv", allProjectSrcFiles);
        runCommandLine({ "./utbot", "run", "--project-path", suitePath, "--build-dir", buildDirectoryName, "project" });
        checkResultsDirectory();
        testUtils::checkExecutionStatsCSV(suitePath / resultsDirectoryName / "execution-stats.csv", allProjectSrcFiles);
    }

    TEST_F(CLI_Test, Run_File_Tests) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath,
                         "--build-dir", buildDirectoryName, "file",
                         "--file-path", suitePath / "basic_functions.c" });
        checkTestDirectory({ basic_functions_tests_cpp, basic_functions_tests_h });
        testUtils::checkGenerationStatsCSV(suitePath / resultsDirectoryName / "generation-stats.csv",
                                           {"basic_functions.c"});
        runCommandLine({ "./utbot", "run", "--project-path", suitePath, "--build-dir", buildDirectoryName, "file",
                         "--file-path", getTestDirectory() / basic_functions_tests_cpp });
        testUtils::checkExecutionStatsCSV(suitePath / resultsDirectoryName / "execution-stats.csv",
                                          {"basic_functions.c"});
        checkResultsDirectory();
    }

    TEST_F(CLI_Test, Run_Specific_Test) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath,
                         "--build-dir", buildDirectoryName, "file",
                         "--file-path", suitePath / "basic_functions.c" });
        checkTestDirectory({ basic_functions_tests_cpp, basic_functions_tests_h });
        runCommandLine({ "./utbot", "run", "--project-path", suitePath,
                         "--build-dir", buildDirectoryName, "test",
                         "--file-path", getTestDirectory() / basic_functions_tests_cpp,
                         "--test-suite", "regression", "--test-name", " max__test_1" });
        checkResultsDirectory();
    }

    TEST_F(CLI_Test, All_Command_Tests) {
        runCommandLine({ "./utbot", "all", "--project-path", suitePath, "--build-dir", buildDirectoryName });
        checkTestDirectory(allProjectTestFiles);
        checkResultsDirectory();
        testUtils::checkGenerationStatsCSV(suitePath / resultsDirectoryName / "generation-stats.csv", allProjectSrcFiles);
        testUtils::checkExecutionStatsCSV(suitePath / resultsDirectoryName / "execution-stats.csv", allProjectSrcFiles);
    }

    TEST_F(CLI_Test, Target_Option_Tests) {
        runCommandLine({ "./utbot", "generate", "--project-path", suitePath, "--build-dir",
                         buildDirectoryName, "file", "--file-path",
                         suitePath / "basic_functions.c", "--target", "cli" });
        checkTestDirectory({ basic_functions_tests_cpp, basic_functions_tests_h });
    }
}
