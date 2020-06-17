#pragma once
#include <fstream>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
namespace hscp {
	class SematicLoader {
		using production = std::pair<std::string, std::list<std::string>>;
		using behavior = std::pair<std::string, std::vector<std::string>>;
	private:
		std::ifstream fin; // file stream
		std::map<production, behavior> sematic;

		void load() {
			std::string line;
			while (std::getline(fin, line)) {
				std::string n, p, t, b;
				std::list<std::string> ps;
				std::vector<std::string> parameters;
				{
					std::stringstream ss(line);
					std::getline(ss, n, '-');
					ss.get();
					ss >> std::ws;

					
					while (ss >> p) {
						ps.push_back(p);
					}
				}
				std::getline(fin, line);
				{
					std::stringstream ss(line);
					std::getline(ss, t, '=');
					ss >> std::ws;

					ss >> b;
					while (ss >> p) {
						parameters.push_back(p);
					}
					std::getline(fin, line); 
				}
				
				sematic.emplace(std::make_pair(n, ps), std::make_pair(b, parameters));
			}
		}
	public:
		SematicLoader() {
#ifdef _DEBUG
			constexpr auto route = "Data\\sematic.txt";
#else
			constexpr auto route = "sematic.txt";
#endif

			if (!std::filesystem::exists(route)) {
				std::cout << "Sematic Definations not Found.\n";
			}
			fin.open(route, std::ios::in | std::ios::_Nocreate);
			load();
		}

		const std::map<production, behavior>& GetSematic() const {
			return sematic;
		}
	};
}