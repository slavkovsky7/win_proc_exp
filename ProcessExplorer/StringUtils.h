#include <vector>
#include <string>

class StringUtils {
public:
	static std::string Tolower(std::string in);
	static std::vector<std::string> ReadFile(const std::string& filePath);
	static std::vector<std::string> ReadStream(const std::string& content);
	static std::string JoinStrings(const std::vector<std::string>& strVec);
};