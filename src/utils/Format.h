#include <string>


/**
   * @brief Convert CFL from ft to FL format (e.g., 4000 ft -> FL040)
   * @param value CFL value as a string (e.g., "4000")
   * @return CFL formatted string (e.g., "FL040")
   */
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