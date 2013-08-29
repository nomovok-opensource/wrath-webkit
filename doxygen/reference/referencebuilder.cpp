#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <cstdio>

const char include[] = "@include ";
const char virt[] = "@virtual ";
std::string virtclass;

std::vector<std::string> build(std::string line, std::string prefix);
std::vector<std::string> build_file(std::string filename, std::string prefix = "");

std::vector<std::string> build_file(std::string filename, std::string prefix)
{
    std::ifstream input(filename.c_str());
    if (!input || !input.is_open()) {
        throw std::runtime_error(std::string("Error: Cannot open input file ") + filename);
    }

    std::vector<std::string> ret;
    std::string line;
    while (std::getline(input, line)) {
        std::vector<std::string> n(build(line, prefix));
        ret.insert(ret.end(), n.begin(), n.end());
    }

    return ret;
}

std::vector<std::string> build(std::string line, std::string prefix)
{
    std::string::size_type inclpos = line.find(include);
    std::string::size_type virtpos = line.find(virt);
    if (inclpos == std::string::npos && virtpos == std::string::npos) {
        return std::vector<std::string>(1, prefix + line + '\n');
    }

    if (inclpos != std::string::npos) {
        std::string newpre = prefix + line.substr(0, inclpos);
        std::string newfile = line.substr(inclpos + sizeof(include) - 1);
        return build_file(newfile, newpre);
    }

    if (virtpos == std::string::npos) {
        throw std::runtime_error("Should not be reached");
    }

    std::string newpre = prefix + line.substr(0, virtpos);
    std::string funcname = line.substr(virtpos + sizeof(virt) - 1);
    std::string newfile = virtclass + "." + funcname;
    return build_file(newfile, newpre);        
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " input\n";
        return 1;
    }

    std::string inputfile(argv[1]);
    virtclass = inputfile.substr(0, inputfile.find('.'));

    try {
        std::vector<std::string> output(build_file(argv[1]));
        std::copy(output.begin(), output.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
    }
    catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
