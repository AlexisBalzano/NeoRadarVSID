#include <string>

std::string formatCFL(const std::string& value) {
    if (value.length() <= 2) {
        return "---";
    }
	std::string cfl = value.substr(0, value.length() - 2);
    
    if (cfl.length() == 2) {
        return "0" + cfl;
	}
    return cfl;
}