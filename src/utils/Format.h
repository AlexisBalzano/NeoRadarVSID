#include <string>

/**
	* @brief Check if a string is numeric
	* @param str Input string
	* @return true if the string is numeric, false otherwise
    */
bool isNumeric(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

/**
   * @brief Convert CFL from ft to FL format (e.g., 7000 ft -> 070)
   * @param value CFL value as a string (e.g., "7000")
   * @return CFL formatted string (e.g., "070")
   */
std::string formatCFL(const std::string& value, const int& transAlt) {
    if (value.length() <= 2 || !isNumeric(value)) {
        return "---";
    }
	std::string cfl = value.substr(0, value.length() - 2);
    
    if (cfl.length() == 2) {
        if (std::stoi(value) - transAlt <= 0) {
			return "A" + cfl;
        }
        return "0" + cfl;
	}
    return cfl;
}