#include <iostream>
#include <vector>
#include <cmath>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <random>
#include <deque>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <future>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;
using namespace std::chrono;

struct TrailPoint {
    float x, y;
    float alpha; // Fades over time
};

struct Particle {
    float x, y;
    float vx, vy;         // Current velocity (scaled)
    float init_vx, init_vy; // Original velocity
    float size;
    float r, g, b;
    string name;
    bool merged = false;
    int decay_time = 0;
    std::deque<TrailPoint> trail;

    bool operator==(const Particle& other) const {
        return name == other.name &&
           std::fabs(x - other.x) < 0.0001f &&
           std::fabs(y - other.y) < 0.0001f &&
           std::fabs(vx - other.vx) < 0.0001f &&
           std::fabs(vy - other.vy) < 0.0001f &&
           std::fabs(size - other.size) < 0.0001f;
    }
};


const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;
float temperature = 0.5f;
float friction = 0.0f;
high_resolution_clock::time_point start;
std::vector<Particle> particles;
const std::vector<std::pair<std::string, float>> ELEMENT_TYPES = {
     {"H", 10.0f}, {"He", 11.0f}, {"Li", 12.0f}, {"Be", 13.0f}, {"B", 14.0f},
    {"C", 15.0f}, {"N", 16.0f}, {"O", 17.0f}, {"F", 18.0f}, {"Ne", 19.0f},
    {"Na", 20.0f}, {"Mg", 21.0f}, {"Al", 22.0f}, {"Si", 23.0f}, {"P", 24.0f},
    {"S", 25.0f}, {"Cl", 26.0f}, {"Ar", 27.0f}, {"K", 28.0f}, {"Ca", 29.0f},
    {"Sc", 30.0f}, {"Ti", 31.0f}, {"V", 32.0f}, {"Cr", 33.0f}, {"Mn", 34.0f},
    {"Fe", 35.0f}, {"Co", 36.0f}, {"Ni", 37.0f}, {"Cu", 38.0f}, {"Zn", 39.0f},
    {"Ga", 40.0f}, {"Ge", 41.0f}, {"As", 42.0f}, {"Se", 43.0f}, {"Br", 44.0f},
    {"Kr", 45.0f}, {"Rb", 46.0f}, {"Sr", 47.0f}, {"Y", 48.0f}, {"Zr", 49.0f},
    {"Nb", 50.0f}, {"Mo", 51.0f}, {"Tc", 52.0f}, {"Ru", 53.0f}, {"Rh", 54.0f},
    {"Pd", 55.0f}, {"Ag", 56.0f}, {"Cd", 57.0f}, {"In", 58.0f}, {"Sn", 59.0f},
    {"Sb", 60.0f}, {"Te", 61.0f}, {"I", 62.0f}, {"Xe", 63.0f}, {"Cs", 64.0f},
    {"Ba", 65.0f}, {"La", 66.0f}, {"Ce", 67.0f}, {"Pr", 68.0f}, {"Nd", 69.0f},
    {"Pm", 70.0f}, {"Sm", 71.0f}, {"Eu", 72.0f}, {"Gd", 73.0f}, {"Tb", 74.0f},
    {"Dy", 75.0f}, {"Ho", 76.0f}, {"Er", 77.0f}, {"Tm", 78.0f}, {"Yb", 79.0f},
    {"Lu", 80.0f}, {"Hf", 81.0f}, {"Ta", 82.0f}, {"W", 83.0f}, {"Re", 84.0f},
    {"Os", 85.0f}, {"Ir", 86.0f}, {"Pt", 87.0f}, {"Au", 88.0f}, {"Hg", 89.0f},
    {"Tl", 90.0f}, {"Pb", 91.0f}, {"Bi", 92.0f}, {"Po", 93.0f}, {"At", 94.0f},
    {"Rn", 95.0f}, {"Fr", 96.0f}, {"Ra", 97.0f}, {"Ac", 98.0f}, {"Th", 99.0f},
    {"Pa", 100.0f}, {"U", 101.0f}, {"Np", 102.0f}, {"Pu", 103.0f}, {"Am", 104.0f},
    {"Cm", 105.0f}, {"Bk", 106.0f}, {"Cf", 107.0f}, {"Es", 108.0f}, {"Fm", 109.0f},
    {"Md", 110.0f}, {"No", 111.0f}, {"Lr", 112.0f}, {"Rf", 113.0f}, {"Db", 114.0f},
    {"Sg", 115.0f}, {"Bh", 116.0f}, {"Hs", 117.0f}, {"Mt", 118.0f}, {"Ds", 119.0f},
    {"Rg", 120.0f}, {"Cn", 121.0f}, {"Nh", 122.0f}, {"Fl", 123.0f}, {"Mc", 124.0f},
    {"Lv", 125.0f}, {"Ts", 126.0f}, {"Og", 127.0f}
};
const std::vector<std::pair<std::string, float>> FUNDAMENTAL_PARTICLES = {
    {"Up quark", 10.0f},
    {"Down quark", 10.0f},
    {"Charm quark", 10.0f},
    {"Strange quark", 10.0f},
    {"Top quark", 10.0f},
    {"Bottom quark", 10.0f},

    // Leptons
    {"Electron", 10.0f},
    {"Muon", 10.0f},
    {"Tau", 10.0f},
    {"Electron neutrino", 10.0f},
    {"Muon neutrino", 10.0f},
    {"Tau neutrino", 10.0f},

    // Gauge Bosons
    {"Photon", 10.0f},
    {"Gluon", 10.0f},
    {"W boson", 10.0f},
    {"Z boson", 10.0f},
    {"Graviton", 10.0f},

    // Higgs Boson
    {"Higgs boson", 10.0f},

    // Baryons
    {"Proton", 10.0f},
    {"Neutron", 10.0f},

    // Antiparticles (mass = particle's mass)
    {"Positron (anti-electron)", 10.0f},
    {"Electron antineutrino", 10.0f},
    {"Antiproton", 10.0f},
    {"Antineutron", 10.0f}
};

string reactionOutput(Particle& a, Particle& b) {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> reactions = {
        {"Li", {{"Al", "LiAl"}, {"Br", "LiBr"}, {"Cl", "LiCl"}, {"F", "LiF"}, {"H", "LiH"},
                {"I", "LiI"}, {"Mg", "LiMg"}, {"Li", "Li2"}}},
        {"Be", {{"O", "BeO"}, {"S", "BeS"}, {"Se", "BeSe"}, {"Te", "BeTe"}, {"O2", "BeO2"}}},
        {"B", {{"N", "BN"}, {"P", "BP"}, {"As", "BAs"}, {"S", "BS"}, {"Si", "BSi"}, {"Fe", "FeB"},
               {"Ni", "NiB"}, {"Co", "CoB"}}},
        {"C", {{"N", "CN"}, {"C", "C2"}, {"Si", "SiC"}, {"Ti", "TiC"}, {"Zr", "ZrC"}, {"Nb", "NbC"},
               {"W", "WC"}, {"V", "VC"}}},
        {"N", {{"N", "N2"}, {"Si", "SiC"}, {"Ti", "TiC"}, {"Zr", "ZrC"}, {"Nb", "NbC"}, {"W", "WC"},
               {"V", "VC"}, {"Al", "AlN"}, {"Cr", "CrN"}, {"Ga", "GaN"}, {"In", "InN"}, {"Sc", "ScN"},
               {"Y", "YN"}}},
        {"H2", {{"O2", "H2O"}, {"Ba", "BaH2"}, {"Be", "BeH2"}, {"Fe", "FeH2"}, {"Ca", "CaH2"},
                {"Mg", "MgH2"}, {"S", "H2S"}, {"Se", "H2Se"}, {"Te", "H2Te"}, {"Zn", "ZnH2"}}},
        {"O2", {{"C", "CO2"}, {"S", "SO2"}, {"Se", "SeO2"}, {"Te", "TeO2"}, {"Si", "SiO2"}, {"Ti", "TiO2"},
        {"V", "VO2"}, {"Mn", "MnO2"}, {"O", "O3"}}},
        {"N2", {{"O2", "NO"}, {"O2", "NO2"}, {"O2", "N2O"}}},
        {"Ba", {{"H2", "BaH2"}}},
        {"Ca", {{"H2", "CaH2"}}}
,{"Fe", {{"Fe", "Fe2"}, {"H2", "FeH2"},
{"F2", "FeF2"},
{"Cl2", "FeCl2"},
{"Br2", "FeBr2"},
{"I2", "FeI2"},
{"N", "FeN"},
{"B", "FeB"},
{"Al", "FeAl"},
{"Cu", "CuFe"}}},
{"Fe2", {{"Zn", "ZnFe2"},
{"Ni", "NiFe2"}}},
{"Mg", {{"H2", "MgH2"},
{"O", "MgO"},
{"F2", "MgF2"},
{"Cl2", "MgCl2"},
{"Br2", "MgBr2"},
{"I2", "MgI2"},
{"B2", "MgB2"},
{"Al2", "MgAl2"},
{"Cu", "CuMg"},
    {"Mg", "Mg2"},
{"Na", "NaMg"},
{"K", "KMg"},
{"Rb", "RbMg"},
{"Cs", "CsMg"}}},
{"Mg2", {{"Zn", "ZnMg2"},
{"Ni", "NiMg2"}}},
        {"O", {{"H2", "H2O"}, {"O", "O2"}, {"F2", "OF2"}, {"Cl2", "OCl2"}, {"Br2", "OBr2"}, {"O", "OI2"},
        {"C", "CO"}, {"Mg", "MgO"}, {"Ca", "CaO"}, {"Se", "SeO2"}, {"Fe", "FeO"}, {"Cu", "CuO"}, {"Zn", "ZnO"},
        {"Ni", "NiO"}, {"Mn", "MnO"}}},
{"S", {{"S", "S2"}, {"H2", "H2S"},
{"O2", "SO2"},
{"F2", "SF2"},
{"Cl2", "SCl2"},
{"Br2", "SBr2"},
{"I2", "SI2"},
{"N", "NS"},
{"B", "BS"},
{"Cu", "CuS"},
{"Zn", "ZnS"},
{"Ni", "NiS"},
{"Na2", "Na2S"},
{"K2", "K2S"},
{"Rb2", "Rb2S"},
{"Cs2", "Cs2S"},
{"Li2", "Li2S"},
{"Mg", "MgS"},
{"Ca", "CaS"},
{"Sr", "SrS"},
{"Ba", "BaS"}}},
        {"S2", {{"C", "CS2"}}},
{"Se", {{"H2", "H2Se"},
{"O2", "SeO2"},
{"F2", "SeF2"},
{"Cl2", "SeCl2"},
{"Br2", "SeBr2"},
{"I2", "SeI2"},
{"N2", "SeN2"},
{"Se", "Se2"},
{"B", "BSe"},
{"Cu", "CuSe"},
{"Zn", "ZnSe"},
{"Ni", "NiSe"},
{"Na2", "Na2Se"},
{"K2", "K2Se"},
{"Rb2", "Rb2Se"},
{"Cs2", "Cs2Se"},
{"Li2", "Li2Se"},
{"Mg", "MgSe"},
{"Ca", "CaSe"},
{"Sr", "SrSe"},
{"Ba", "BaSe"}}},
        {"Se2", {{"C", "CSe2"}}},
{"Te", {{"Te", "Te2"}, {"H2", "H2Te"},
{"O2", "TeO2"},
{"N2", "TeN2"},
{"B", "BTe"},
{"Cu", "CuTe"},
{"Zn", "ZnTe"},
{"Ni", "NiTe"},
{"Na2", "Na2Te"},
{"K2", "K2Te"},
{"Rb2", "Rb2Te"},
{"Cs", "Cs2Te"},
{"Li2", "Li2Te"},
{"Mg", "MgTe"},
{"Ca", "CaTe"},
{"Sr", "SrTe"},
{"Ba", "BaTe"}}},
        {"Te2", {{"C", "CTe2"}}},
{"Zn", {{"H", "ZnH2"},
{"O", "ZnO"},
{"F2", "ZnF2"},
{"Cl2", "ZnCl2"},
{"Br2", "ZnBr2"},
{"I2", "ZnI2"},
{"B2", "ZnB2"},
{"Al2", "ZnAl2"},
{"Cu", "CuZn"},
{"Ni", "NiZn"},
{"Na", "Na2Zn"},
{"K2", "K2Zn"},
{"Rb2", "Rb2Zn"},
{"Cs2", "Cs2Zn"},
{"Li", "LiZn"},
{"Mg", "MgZn"},
{"Ca", "CaZn"}}},
{"Na", {{"Na", "Na2"}, {"H", "NaH"},
{"F", "NaF"},
{"Cl", "NaCl"},
{"Br", "NaBr"},
{"I", "NaI"},
{"B", "NaB"},
{"Al", "NaAl"},
{"Cu", "CuNa"},
{"K", "NaK"},
{"Rb", "NaRb"},
{"Cs", "NaCs"},
{"Li", "NaLi"}}},
{"Na2", {{"O", "Na2O"}, {"Zn", "ZnNa2"},
{"Ni", "NiNa2"},{"He", "Na2He"}, {"O", "Na2O"}}},
        {"F", {{"F", "F2"}, {"H", "HF"}, {"Cl", "FCl"}, {"Br", "FBr"}, {"I", "FI"}, {"Cu", "CuF"},
        {"Na", "NaF"}, {"K", "KF"}, {"Li", "LiF"}, {"Rb", "RbF"}, {"Cs", "CsF"}}},
        {"F2", {{"O", "OF2"}, {"Zn", "ZnF2"}, {"Ni", "NiF2"}, {"Mn", "MnF2"}}},
        {"Cl", {{"Cl", "Cl2"}, {"H", "HCl"}, {"O2", "ClO2"}, {"F", "ClF"}, {"Br", "ClBr"}, {"I", "Cl"},
        {"Cu", "CuCl"}, {"Na", "NaCl"}, {"K", "KCl"}, {"Li", "LiCl"}, {"Rb", "RbCl"}, {"Cs", "CsCl"}}},
        {"Cl2", {{"O", "OCl2"}, {"S", "Cl2S"}, {"Se", "Cl2Se"}, {"Te", "Cl2Te"}, {"Cl", "Cl2"},
        {"Zn", "ZnCl2"}, {"Ni", "NiCl2"}, {"Mn", "MnCl2"}}},
{"Br", {{"Br", "Br2"}, {"Br", "Br2"}, {"H", "HBr"},
{"F", "BrF"},
{"Cl", "ClBr"},
{"I", "BrI"},
{"N", "BrNO"},
{"Cu", "CuBr"},
{"Na", "NaBr"},
{"K", "KBr"},
{"Li", "LiBr"},
{"Rb", "RbBr"},
{"Cs", "CsBr"}}},
        {"Br2", {{"O", "OBr2"}, {"Zn", "ZnBr2"},
{"Ni", "NiBr2"}}},
{"I", {{"H", "HI"},
{"F", "IF"},
{"Cl", "ClI"},
{"Br", "BrI"},
{"Cu", "CuI"},
{"Na", "NaI"},
{"K", "KI"},
{"Li", "LiI"},
{"Rb", "RbI"},
{"Cs", "CsI"}}},
        {"I2", {{"O", "OI2"}, {"Zn", "ZnI2"},
{"Ni", "NiI2"},
}},
{"Li2", {{"O", "Li2O"},  {"Zn", "ZnLi2"},
{"Ni", "NiLi2"}}},
{"K", { {"H", "KH"},
{"F", "KF"},
{"Cl", "KCl"},
{"Br", "KBr"},
{"I", "KI"},
{"B", "KB"},
{"Al", "KAl"},
{"Cu", "CuK"},
{"Na", "NaK"},
{"Li", "LiK"},
{"Rb", "RbK"},
{"Cs", "CsK"}}},
        {"K2", {{"O", "K2O"}, {"Zn", "ZnK2"},
{"Ni", "NiK2"},
}},
{"Rb", {{"H", "RbH"},
{"F", "RbF"},
{"Cl", "RbCl"},
{"Br", "RbBr"},
{"I", "RbI"},
{"B", "RbB"},
{"Al", "RbAl"},
{"Cu", "CuRb"},
{"Na", "NaRb"},
{"Li", "LiRb"},
{"K", "KRb"},
{"Cs", "CsRb"}}},
{"Rb2", {{"O", "Rb2O"}, {"Zn", "ZnRb2"},
{"Ni", "NiRb2"},}},
{"Cs", {{"H", "CsH"},
{"F", "CsF"},
{"Cl", "CsCl"},
{"Br", "CsBr"},
{"I", "CsI"},
{"B", "CsB"},
{"Al", "CsAl"},
{"Cu", "CuCs"},
{"Na", "NaCs"},
{"Li", "LiCs"},
{"K", "KCs"},
{"Rb", "RbCs"}}},
        {"Cs2", {{"O", "Cs2O"}, {"Zn", "ZnCs2"},
{"Ni", "NiCs2"}
        }},
        {"P", {
    {"N", "PN"},
    {"B", "PB"},
    {"Al", "PAl"},
    {"Cu", "CuP"},
    {"Zn", "ZnP"},
    {"Ni", "NiP"},
    {"Na", "NaP"},
    {"K", "KP"},
    {"Rb", "RbP"},
    {"Cs", "CsP"}}},
    {"S", {{"H2", "H2S"},
    {"O2", "SO2"},
    {"Cl2", "SCl2"},
    {"Br2", "SBr2"},
    {"I2", "SI2"},
    {"N2", "SN2"},
    {"B", "SB"},
    {"Al2", "SAl2"},
    {"Cu", "CuS"},
    {"Zn", "ZnS"},
    {"Ni", "NiS"},
    {"Na2", "Na2S"},
    {"K2", "K2S"},
    {"Rb2", "Rb2S"},
    {"Cs2", "Cs2S"}}},
    {"Sr", {{"H2", "SrH2"},
        {"O", "SrO"},
        {"F2", "SrF2"},
        {"Cl2", "SrCl2"},
        {"Br2", "SrBr2"},
        {"I2", "SrI2"},
        {"B", "SrB"},
        {"Al2", "SrAl2"},
        {"Cu", "CuSr"},
        {"Na", "NaSr"},
        {"K", "KSr"},
        {"Rb", "RbSr"},
        {"Cs", "CsSr"},
    }},
    {"Sr2", {{"Zn", "ZnSr2"},
    {"Ni", "NiSr2"}}},
    {"Sr", {{"Sr", "Sr2"}}},
    {"Ra", {{"H", "RaH2"},
    {"O", "RaO"},
    {"F2", "RaF2"},
    {"Cl2", "RaCl2"},
    {"Br2", "RaBr2"},
    {"I2", "RaI2"},
    {"B", "RaB"},
    {"Al2", "RaAl2"},
    {"Cu", "CuRa"},
    {"Na", "NaRa"},
    {"K", "KRa"},
    {"Rb", "RbRa"},
    {"Cs", "CsRa"}}},
    {"Ra2", {{"Ra", "Ra2"},{"Zn", "ZnRa2"},
    {"Ni", "NiRa2"}}},
    {"Fr", {{"H", "FrH"},
    {"F", "FrF"},
    {"Cl", "FrCl"},
    {"Br", "FrBr"},
    {"I", "FrI"},
    {"B", "FrB"},
    {"Al", "FrAl"},
    {"Cu", "CuFr"},
    {"Na", "NaFr"},
    {"Li", "LiFr"},
    {"K", "KFr"},
    {"Rb", "RbFr"},
    {"Cs", "CsFr"}}},
    {"Fr", {{"Fr", "Fr2"}, {"O", "Fr2O"},
    {"Zn", "ZnFr2"},
    {"Ni", "NiFr2"}}},
    {"Sc", {{"H2", "ScH2"},
        {"N", "ScN"},
        {"B", "ScB"},
        {"Al", "ScAl"},
        {"Cu", "CuSc"}}},
    {"Sc2", {{"Sc", "Sc2"}, {"Zn", "ZnSc2"},
        {"Ni", "NiSc2"}}},
    {"Ra", {
        {"N", "YN"},
        {"B", "YB"},
        {"Al2", "YAl2"}}},
    {"Y", {{"Y", "Y2"},     {"Zn", "ZnY2"},
        {"Ni", "NiY2"}}},
    {"Ti", { {"H2", "TiH2"},
        {"O2", "TiO2"},
        {"N", "TiN"},
        {"B2", "TiB2"},
        {"Cu", "CuTi"},
        {"Zn", "ZnTi"},
        {"Ni", "NiTi"}}},
    {"V", {{"V", "V2"}, {"H2", "VH2"},
    {"N", "VN"},
    {"B2", "VB2"},
    {"Cu", "CuV"}}},
    {"V", {{"Zn", "ZnV2"},
    {"Ni", "NiV2"}}},
    {"Cr", {{"Cr", "Cr2"}, {"H2", "CrH2"},
        {"N", "CrN"},
        {"B", "CrB"},
        {"Al", "CrAl"},
        {"Cu", "CuCr"}}},
    {"Cr2", {{"Zn", "ZnCr2"},
    {"Ni", "NiCr2"}}},
    {"Mn", {{"Mn", "Mn2"}, {"H2", "MnH2"},
    {"O2", "MnO2"},
    {"Cl2", "MnCl2"},
    {"Br2", "MnBr2"},
    {"I2", "MnI2"},
    {"N", "MnN"},
    {"B", "MnB"},
    {"Al", "MnAl"},
    {"Cu", "CuMn"}}},
    {"Mn2", {{"Zn", "ZnMn2"},
    {"Ni", "NiMn2"}}},
    {"Co", { {"Co", "Co2"}, {"H2", "CoH2"},
        {"O", "CoO"},
        {"F2", "CoF2"},
        {"Cl2", "CoCl2"},
        {"Br2", "CoBr2"},
        {"I2", "CoI2"},
        {"N", "CoN"},
        {"B", "CoB"},
        {"Al", "CoAl"},
        {"Cu", "CuCo"},
        {"Na", "NaCo"},
        {"K", "KCo"},
        {"Rb", "RbCo"},
        {"Cs", "CsCo"}}},
    {"Co2", {    {"Zn", "ZnCo2"},
    {"Ni", "NiCo2"}}},
    {"Cu", {{"H2", "CuH2"},
    {"O", "CuO"},
    {"F2", "CuF2"},
    {"Cl2", "CuCl2"},
    {"Br2", "CuBr2"},
    {"I2", "CuI2"},
    {"N", "CuN"},
    {"B", "CuB"},
    {"Al", "CuAl"},
    {"Zn", "CuZn"},
    {"Ni", "CuNi"},
    {"Na", "NaCu"},
    {"K", "KCu"},
    {"Rb", "RbCu"},
    {"Cs", "CsCu"}}},
    {"Zr", {{"H2", "ZrH2"},
    {"O2", "ZrO2"},
    {"N", "ZrN"}, {"Zr", "Zr2"},
    {"B2", "ZrB2"},
    {"Cu", "CuZr"}}},
    {"Zr2", {{"Zn", "ZnZr2"},
    {"Ni", "NiZr2"}}},
    {"Nb", {{"H2", "NbH2"},
    {"N", "NbN"},
    {"B2", "NbB2"},
    {"Cu", "CuNb"},
        {"Nb", "Nb2"}}},
    {"Nb2", {{"Zn", "ZnNb2"},
    {"Ni", "NiNb2"}}},
    {"Mo", {{"H2", "MoH2"},
    {"N", "MoN"},
    {"B2", "MoB2"},
    {"Cu", "CuMo"}}},
    {"Mo2", {{"Zn", "ZnMo2"},
    {"Ni", "NiMo2"}}},
    {"Tc", {{"H", "TcH2"},
    {"O2", "TcO2"},
    {"N", "TcN"},
    {"B", "TcB"},
    {"Cu", "CuTc"}, {"Tc", "Tc2"}}},
    {"Tc2", {{"Zn", "ZnTc2"},
    {"Ni", "NiTc2"}}},
    {"Ru", {{"H2", "RuH2"},
    {"O2", "RuO2"},
    {"N", "RuN"},
    {"B", "RuB"},
    {"Cu", "CuRu"},
    {"Ru", "Ru2"}}},
    {"Ru2", {{"Zn", "ZnRu2"},
    {"Ni", "NiRu2"}}},
    {"Rh", {{"H2", "RhH2"},
    {"N", "RhN"},
    {"B", "RhB"},
    {"Cu", "CuRh"},
    {"Rh", "Rh2"}}},
    {"Rh2", {
        {"Zn", "ZnRh2"},
        {"Ni", "NiRh2"}}},
    {"Pd", {{"H2", "PdH2"},
    {"O", "PdO"},
    {"F2", "PdF2"},
    {"Cl2", "PdCl2"},
    {"Br2", "PdBr2"},
    {"I2", "PdI2"},
    {"N", "PdN"},
    {"B", "PdB"},
    {"Cu", "CuPd"}, {"Pd", "Pd2"}}},
    {"Pd2", {{"Zn", "ZnPd2"},
    {"Ni", "NiPd2"}}},
    {"Pt", {{"H2", "PtH2"},
    {"O2", "PtO2"},
    {"N", "PtN"},
    {"B", "PtB"},
    {"Cu", "CuPt"},{"Pt", "Pt2"}}},
    {"Pt2", {{"Zn", "ZnPt2"},
    {"Ni", "NiPt2"}}},
    {"Au", {{"N", "AuN"},
    {"B", "AuB"},
    {"Cu", "CuAu"}, {"Au", "Au2"}}},
    {"Au2", {{"Zn", "ZnAu2"},
    {"Ni", "NiAu2"}}},
    {"Hg", {{"H2", "HgH2"},
    {"O", "HgO"},
    {"F2", "HgF2"},
    {"Cl2", "HgCl2"},
    {"Br2", "HgBr2"},
    {"I2", "HgI2"},
    {"N", "HgN"},
    {"B2", "HgB2"},
    {"Al2", "HgAl2"},
    {"Cu", "CuHg"},
    {"Hg", "Hg2"}}},
    {"Hg2", {{"Zn", "ZnHg2"},
    {"Ni", "NiHg2"}}},
    {"Tl", {{"H", "TlH"},
    {"N", "TlN"},
    {"B", "TlB"},
    {"Cu", "CuTl"},
    {"Tl", "Tl2"}}},
    {"Tl2", {{"Zn", "ZnTl2"},
    {"Ni", "NiTl2"}}},
    {"Pb", {{"H2", "PbH2"},
    {"O", "PbO"},
    {"F2", "PbF2"},
    {"Cl2", "PbCl2"},
    {"Br2", "PbBr2"},
    {"I2", "PbI2"},
    {"N", "PbN"},
    {"B2", "PbB2"},
    {"Cu", "CuPb"}, {"Pb", "Pb2"}}},
    {"Pb2", {{"Zn", "ZnPb2"},
    {"Ni", "NiPb2"}}},
    {"Bi", {
    {"N", "BiN"},
    {"B", "BiB"},
    {"Cu", "CuBi"}, {"Bi", "Bi2"}}},
    {"Bi2", {{"Zn", "ZnBi2"},
    {"Ni", "NiBi2"}}},
    {"Po", {{"H2", "PoH2"},
    {"O2", "PoO2"},
    {"F2", "PoF2"},
    {"Cl2", "PoCl2"},
    {"Br2", "PoBr2"},
    {"I2", "PoI2"},
    {"N", "PoN"},
    {"B", "PoB"},
    {"Cu", "CuPo"}, {"Po", "Po2"}}},
    {"Po2", {{"Zn", "ZnPo2"},
    {"Ni", "NiPo2"}}},
    {"At", {{"H", "AtH"}, {"At", "At2"},
    {"N", "AtN"},
    {"B", "AtB"},
    {"Cu", "CuAt"}}},
    {"At2", {{"O", "At2O"}, {"Zn", "ZnAt2"},
    {"Ni", "NiAt2"}}},
    {"At", {
            {"H", "HAt"},
            {"Li", "LiAt"},
            {"Na", "NaAt"},
            {"K", "KAt"},
            {"Rb", "RbAt"},
            {"Cs", "CsAt"},
            {"Fr", "FrAt"},
            {"Mg", "MgAt2"},
            {"Ca", "CaAt2"},
            {"Sr", "SrAt2"},
            {"Ba", "BaAt2"},
            {"Ra", "RaAt2"},
            {"Tl", "TlAt"},
            {"Pb", "PbAt2"},
            {"O", "OAt2"},
            {"S", "SAt2"},
            {"Se", "SeAt2"},
            {"Te", "TeAt2"},
            {"F", "FAt"},
            {"Cl", "ClAt"},
            {"Br", "BrAt"},
            {"I", "IAt"},
            {"At", "At2"}}},
    {"Rf", {
            {"O", "RfO2"}}},
    {"Db", {
        {"O", "DbO2"}}},
    {"Mt", {
            {"O", "MtO2"}
    }},
    {"Ds", {
            {"O", "DsO2"}
    }},
    {"Rg", {
            {"O", "RgO"}
    }},
    {"Cn", {
            {"F", "CnF2"},
            {"Cl", "CnCl2"},
            {"Br", "CnBr2"},
            {"I", "CnI2"},
            {"O", "CnO"}
    }},
    {"Nh", {
            {"O", "NhO"}
    }},
    {"Fl", {
            {"O", "FlO2"}
    }},
    {"Mc", {
            {"I", "McI3"},
            {"O", "McO"}
    }},
    {"Lv", {
            {"F", "LvF2"},
            {"Cl", "LvCl2"},
            {"Br", "LvBr2"},
            {"I", "LvI2"},
            {"O", "LvO2"}
    }},
    {"Og", {
            {"F", "OgF2"},
            {"Cl", "OgCl2"},
            {"Br", "OgBr2"},
            {"I", "OgI2"},
            {"O", "OgO2"}}}

    };

    // Try finding a reaction from a to b
    auto it = reactions.find(a.name);
    if (it != reactions.end()) {
        auto inner_it = it->second.find(b.name);
        if (inner_it != it->second.end()) {
            return inner_it->second;  // found a reaction!
        }
    }

    // Try the reverse reaction from b to a
    it = reactions.find(b.name);
    if (it != reactions.end()) {
        auto inner_it = it->second.find(a.name);
        if (inner_it != it->second.end()) {
            return inner_it->second;  // found a reaction!
        }
    }

    // No reaction found
    return "";
}

// Minimum separation distance between particles
float minimumSeparation(const Particle& a, const Particle& b) {
    return a.size + b.size;
}

// Helper function to check if a particle overlaps with any others
bool isOverlapping(const Particle& newParticle, const std::vector<Particle>& particles) {
    for (const auto& p : particles) {
        float dx = newParticle.x - p.x;
        float dy = newParticle.y - p.y;
        float distanceSquared = dx * dx + dy * dy;
        float minDist = minimumSeparation(newParticle, p);
        if (distanceSquared < minDist * minDist) {
            return true; // Overlap detected
        }
    }
    return false; // No overlap
}

void generateParticle(string particle_name) {
    // for (auto particle : p) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distX(0.0f, WINDOW_WIDTH);
    std::uniform_real_distribution<float> distY(0.0f, WINDOW_HEIGHT);

    Particle newParticle;

    // Set properties (you can adjust these!)
    newParticle.size = 10.0f;
    newParticle.init_vx = 0.0f;
    newParticle.init_vy = 0.0f;
    newParticle.vx = 0.0f;
    newParticle.vy = 0.0f;
    newParticle.r = 1.0f;
    newParticle.g = 1.0f;
    newParticle.b = 1.0f;
    newParticle.name = particle_name;
    newParticle.merged = false;

    // Try finding a non-overlapping position
    while (isOverlapping(newParticle, particles)) {
        newParticle.x = distX(gen);
        newParticle.y = distY(gen);
    }

    // newParticle.trail.push_back({newParticle.x, newParticle.y, 1.0f});
    particles.push_back(newParticle);
    // }

}

void particle_thread(Particle& particle, int decay_time, int id) {
    while (particle.size >= 92.0f) {
        std::this_thread::sleep_for(std::chrono::seconds(decay_time));
        if (particle.size - ELEMENT_TYPES[1].second < 92.0f) break;

        particle.size -= ELEMENT_TYPES[1].second; // directly modify the particle
    }

        // std::random_device rd;
        // std::mt19937 gen(rd());
        // std::uniform_int_distribution<> dist(0, 3);
        // int selection = dist(gen);
        //
        // std::vector<std::vector<std::pair<std::string, float>>> arr = {
        //     {FUNDAMENTAL_PARTICLES[18], FUNDAMENTAL_PARTICLES[6], FUNDAMENTAL_PARTICLES[22]},
        //     {FUNDAMENTAL_PARTICLES[19], FUNDAMENTAL_PARTICLES[20], FUNDAMENTAL_PARTICLES[21]}
        // };
        //
        //
        // if (selection == 0) {
        // generateParticle({ELEMENT_TYPES[1]});
        // }

        // else if (selection == 1) {
            // generateParticle({FUNDAMENTAL_PARTICLES[18], FUNDAMENTAL_PARTICLES[6], FUNDAMENTAL_PARTICLES[22]});
            // totalNewSize += FUNDAMENTAL_PARTICLES[18].second + FUNDAMENTAL_PARTICLES[6].second + FUNDAMENTAL_PARTICLES[22].second;
        //}
        // else {
        //     generateParticle({FUNDAMENTAL_PARTICLES[19], FUNDAMENTAL_PARTICLES[20], FUNDAMENTAL_PARTICLES[21]});
        //     totalNewSize += FUNDAMENTAL_PARTICLES[19].second + FUNDAMENTAL_PARTICLES[28].second + FUNDAMENTAL_PARTICLES[21].second;
        // }

        // Decay
}

void mergeParticles(Particle& a, Particle& b, string& new_name) {
    // m1 * vi1 + m2 * vi2 = (m1 + m2) * vf
    float totalMass = a.size + b.size;

    // New position (center of mass)
    float newX = (a.x * a.size + b.x * b.size) / totalMass;
    float newY = (a.y * a.size + b.y * b.size) / totalMass;

    // New velocity (momentum conservation)
    float newVx = (a.vx * a.size + b.vx * b.size) / totalMass;
    float newVy = (a.vy * a.size + b.vy * b.size) / totalMass;

    // New size (assuming area is proportional to size^2)
    float newSize = std::sqrt(a.size * a.size + b.size * b.size);

    // Create the new particle
    Particle merged;
    merged.x = newX;
    merged.y = newY;
    merged.init_vx = newVx;
    merged.init_vy = newVy;
    merged.vx = newVx;
    merged.vy = newVy;
    merged.size = newSize;
    merged.r = (a.r + b.r) / 2.0f;
    merged.b = (a.b + b.b) / 2.0f;
    merged.g = (a.g + b.g) / 2.0f;
    merged.name = new_name;
    if (a.name == "H2" || b.name == "H2" || a.name == "O2" || b.name == "O2") {
        merged.merged = false;
    }
    else {
        merged.merged = true;
    }
    merged.trail.push_back({a.x + b.x, a.y + b.y, 2.0f});

    // Add merged particle to list and mark a/b for deletion
    particles.push_back(merged);
}


void resolveCollision(Particle& a, Particle& b) {
    // Compute distance between particles
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float distSq = dx * dx + dy * dy;
    float minDist = a.size + b.size;
    std::vector<size_t> toRemove;

    if (distSq < minDist * minDist) {
        //m1 * vi1 + m2 * vi2 = (m1 + m2) * vf
        // && ((abs(a.vx) + abs(b.vx) >= 7.0f && abs(a.vy) + abs(b.vy) >= 7.0f ||(abs(a.init_vx) + abs(b.init_vx) >= 7.0f && abs(a.init_vy) + abs(b.init_vy) >= 7.0f))
        cout << "a: " << a.merged << endl;
        cout << "b: " << b.merged << endl;
        if ((a.merged == false && b.merged == false) && !reactionOutput(a, b).empty()){
            auto val = reactionOutput(a, b);
            mergeParticles(a, b, val);
        }
        else {
            float r_1 = a.vx - b.vx;
            float r_2 = a.vy - b.vy;
            float c_1 = a.x - b.x;
            float c_2 = a.y - b.y;
            float r_c = r_1 * c_1 + r_2 * c_2;

            float rb_1 = b.vx - a.vx;
            float rb_2 = b.vy - a.vy;
            float cb_1 = b.x - a.x;
            float cb_2 = b.y - a.y;
            float rb_c = rb_1 * cb_1 + rb_2 * cb_2;

            a.init_vx = a.init_vx - (2*b.size/(a.size + b.size)) * (r_c/(c_1 * c_1 + c_2 * c_2)) * c_1;
            a.init_vy = a.init_vy - (2*b.size/(a.size + b.size)) * (r_c/(c_1 * c_1 + c_2 * c_2)) * c_2;
            a.vx -= 0.01;
            a.vy -= 0.01;

            b.init_vx = b.init_vx - (2*a.size/(a.size + b.size)) * (rb_c/(cb_1 * cb_1 + cb_2 * cb_2)) * cb_1;
            b.init_vy = b.init_vy - (2*a.size/(a.size + b.size)) * (rb_c/(cb_1 * cb_1 + cb_2 * cb_2)) * cb_2;
            b.vx -= 0.01;
            b.vy -= 0.01;

        }

        // Separate overlapping particles
        float dist = std::sqrt(distSq);
        float overlap = 0.5f * (minDist - dist + 1.0f);
        float nx = dx / dist;
        float ny = dy / dist;
        a.x -= nx * overlap;
        a.y -= ny * overlap;
        b.x += nx * overlap;
        b.y += ny * overlap;
    }
}

void initParticles(const size_t num, vector<std::pair<std::string, float>> l) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> type_dist(0, l.size() - 1);
    std::uniform_int_distribution<> dist_width(0.0f, static_cast<float>(WINDOW_WIDTH));
    std::uniform_int_distribution<> dist_height(0.0f, static_cast<float>(WINDOW_HEIGHT));
    std::uniform_int_distribution<> dist_v(-6.0f, 6.0f);
    std::uniform_real_distribution<> dist_color(0.0f, 1.0f);
    std::uniform_int_distribution<> decay_time(1, 11);
    std::vector<std::thread> threads;

    for (int i = 0; i < num; ++i) {
        auto& [name, radius] = l[type_dist(gen)];
        Particle p;
        p.size = radius;
        p.name = name;
        p.x = dist_width(gen);
        p.y = dist_height(gen);
        p.init_vx = dist_v(gen);
        p.init_vy = dist_v(gen);
        p.vx = p.init_vx;
        p.vy = p.init_vy;
        p.r = dist_color(gen);
        p.g = dist_color(gen);
        p.b = dist_color(gen);
        if (p.size >= 92) {
            p.decay_time = decay_time(gen);
            threads.push_back(std::thread(particle_thread, std::ref(p),p.decay_time, i + 1));

        }
        particles.push_back(p);

    }

    for (auto& t : threads) {
        t.detach();  // Detach to allow them to run independently
    }
}

void updateParticles() {
    std::vector<Particle> new_particles;
    for (auto& p : particles) {
        // Start a background thread for each particle
        std::thread([&p]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                // Reduce particle size by 10
                if (p.size >= 92.0f) {
                    p.size -= 10.0f;
                    initParticles(1, FUNDAMENTAL_PARTICLES);
                }
            }
        }).detach();
        // Scale velocity with temperature
        p.vx = p.init_vx * temperature;
        p.vy = p.init_vy * temperature;
        // Scale velocity with air resistance
        p.vx *= (1.0f - friction);
        p.vy *= (1.0f - friction);
        p.x += p.vx;
        p.y += p.vy;

        // Add to trail
        float speed = std::sqrt(p.vx * p.vx + p.vy * p.vy);
        p.trail.push_back({p.x, p.y, 1.0f});

        // Lifespan proportional to speed
        size_t maxTrailLength = static_cast<size_t>(std::clamp(speed * 10.0f, 5.0f, 50.0f));
        while (p.trail.size() > maxTrailLength) {
            p.trail.pop_front();
        }

        // Fade trail alpha
        for (auto& pt : p.trail) {
            pt.alpha *= 0.95f; // Fade out
        }

        // Bounce off the window edges
        if (p.x - p.size < 0.0f) {
            p.x = p.size;
            p.init_vx *= -1.0f;
        }
        if (p.x + p.size > WINDOW_WIDTH) {
            p.x = WINDOW_WIDTH - p.size;
            p.init_vx *= -1.0f;
        }

        // Check Y boundaries
        if (p.y - p.size < 0.0f) {
            p.y = p.size;
            p.init_vy *= -1.0f;
        }
        if (p.y + p.size > WINDOW_HEIGHT) {
            p.y = WINDOW_HEIGHT - p.size;
            p.init_vy *= -1.0f;
        }

        // Check collisions between particles
        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = i + 1; j < particles.size(); ++j) {
                resolveCollision(particles[i], particles[j]);
            }
        }
    }
}

void renderParticles() {
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    for (const auto& p : particles) {
        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

        // Draw trail
        for (size_t i = 1; i < p.trail.size(); ++i) {
            auto& prev = p.trail[i - 1];
            auto& curr = p.trail[i];
            ImU32 faded = IM_COL32(p.r * 255, p.g * 255, p.b * 255, static_cast<int>(curr.alpha * 255));
            draw_list->AddLine(ImVec2(prev.x, prev.y), ImVec2(curr.x, curr.y), faded, 1.0f);
        }

        // Draw circle
        ImU32 color = IM_COL32(p.r * 255, p.g * 255, p.b * 255, 255);
        draw_list->AddCircleFilled(ImVec2(p.x, p.y), p.size, color);

        // Draw label
        ImVec2 text_size = ImGui::CalcTextSize(p.name.c_str());
        draw_list->AddText(ImVec2(p.x - text_size.x / 2, p.y - text_size.y / 2), IM_COL32(255, 255, 255, 255), p.name.c_str());
    }

}

ImVec4 getTemperatureColor(float temp) {
    if (temp <= 0.5f) {
        float t = temp / 0.5f;
        return ImVec4(1.0f * (1 - t) + 0.0f * t, 1.0f * (1 - t) + 0.0f * t, 1.0f * (1 - t) + 1.0f * t, 1.0f); // White → Blue
    } else {
        float t = (temp - 0.5f) / 0.5f;
        return ImVec4(0.0f * (1 - t) + 1.0f * t, 0.0f, 1.0f * (1 - t) + 0.0f * t, 1.0f); // Blue → Red
    }
}

ImVec4 getResistanceColor(float resistance) {
    float level = 1.0f - resistance / 0.25f; // 1.0 → 0.0
    return ImVec4(level, level, level, 1.0f);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);  // Set orthographic projection

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark(); // Or Light()

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    cout << "Would you like to simulation element interactions, fundamental particle interactions, or a combination of both?" << endl;
    cout << "Enter either ELEMENT, PARTICLE, or BOTH: " << endl;
    string res;
    cin >> res;

    transform(res.begin(), res.end(), res.begin(), ::toupper);
    while (res != "ELEMENT" and res != "PARTICLE" and res != "BOTH") {
        cout << "Please enter either ELEMENT, PARTICLE, or BOTH: ";
        cin >> res;
        transform(res.begin(), res.end(), res.begin(), ::toupper);
    }

    transform(res.begin(), res.end(), res.begin(), ::tolower);
    cout << "Please enter the number of items to display: ";
    int num;
    cin >> num;
    while (cin.fail()) {
        cin.clear(); // Clear the fail state
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input

        cout << "Please enter a number: ";
        cin >> num;
    }
    if (res == "element") initParticles(num, ELEMENT_TYPES);
    else if (res == "particle") initParticles(num, FUNDAMENTAL_PARTICLES);
    else {
        auto both = ELEMENT_TYPES;
        both.insert(both.end(), ELEMENT_TYPES.begin(), ELEMENT_TYPES.end());
        initParticles(num, both);
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // === Start ImGui Frame ===
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // === ImGui UI Window ===
        ImGui::Begin("Controls");

        // === Temperature Slider ===
        ImVec4 tempColor = getTemperatureColor(temperature);
        ImGui::TextColored(tempColor, "Temperature");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, tempColor);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, tempColor);
        ImGui::SliderFloat("##TempSlider", &temperature, 0.0f, 1.0f);
        ImGui::PopStyleColor(2);

        // === Air Resistance Slider ===
        ImVec4 resistanceColor = getResistanceColor(friction);
        ImGui::TextColored(resistanceColor, "Air Resistance");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, resistanceColor);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, resistanceColor);
        ImGui::SliderFloat("##AirSlider", &friction, 0.0f, 0.25f);
        ImGui::PopStyleColor(2);

        ImGui::End();

        // === Rendering ===
        glClear(GL_COLOR_BUFFER_BIT);

        updateParticles();
        renderParticles();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown(); // or OpenGL3
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}