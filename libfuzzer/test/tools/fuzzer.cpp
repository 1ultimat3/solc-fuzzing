#include <libdevcore/CommonIO.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/ConstantOptimiser.h>
#include <libsolc/libsolc.h>

#include <libdevcore/JSON.h>

#include <boost/program_options.hpp>

#include <string>
#include <sstream>
#include <iostream>

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace po = boost::program_options;

namespace
{

bool quiet = false;

string contains(string const& _haystack, vector<string> const& _needles)
{
	for (string const& needle: _needles)
		if (_haystack.find(needle) != string::npos)
			return needle;
	return "";
}

void runCompiler(string input)
{
	cout << input;
	string outputString(solidity_compile(input.c_str(), nullptr));
	Json::Value output;
	if (!jsonParseStrict(outputString, output))
	{
		cout << "Compiler produced invalid JSON output." << endl;
		abort();
	}
	if (output.isMember("errors"))
		for (auto const& error: output["errors"])
		{
			string invalid = contains(error["type"].asString(), vector<string>{
				"Exception",
				"InternalCompilerError"
			});
			if (!invalid.empty())
			{
				cout << "Invalid error: \"" << error["type"].asString() << "\"" << endl;
				abort();
			}
		}
}

void testCompiler(string const& input, bool optimize)
{
	if (!quiet)
		cout << "Testing compiler " << (optimize ? "with" : "without") << " optimizer." << endl;

	Json::Value config = Json::objectValue;
	config["language"] = "Solidity";
	config["sources"] = Json::objectValue;
	config["sources"][""] = Json::objectValue;
	config["sources"][""]["content"] = input;
	config["settings"] = Json::objectValue;
	config["settings"]["optimizer"] = Json::objectValue;
	config["settings"]["optimizer"]["enabled"] = optimize;
	config["settings"]["optimizer"]["runs"] = 200;

	// Enable all SourceUnit-level outputs.
	config["settings"]["outputSelection"]["*"][""][0] = "*";
	// Enable all Contract-level outputs.
	config["settings"]["outputSelection"]["*"]["*"][0] = "*";

	runCompiler(jsonCompactPrint(config));
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t) {
  string input = string((char *) Data);
  testCompiler(input, false);
  return 0;
}

}
