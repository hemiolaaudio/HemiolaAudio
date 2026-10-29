// Hemiola Audio: cycle definitions (meters, Hindustani talas, Carnatic suladi talas).
// No Rack code in this file, so it can be unit-tested with a plain compiler.
//
// Every definition lists its sections (vibhag in Hindustani music, anga groups in
// Carnatic music) as lengths in beats. The lengths must add up to the beat count.
//
// Sources used to check the definitions (two independent sources per Hindustani tala):
//   Hindustani: ancient-future.com/theka.html and en.wikipedia.org/wiki/Vibhag
//   Carnatic:   karnatik.com/ctaala.shtml and acharyanet.com/all-about-talas-in-carnatic-music
// Carnatic cycles are generated from the published anga rules (laghu = jati beats,
// drutam = 2 beats, anudrutam = 1 beat), not typed in by hand.
#pragma once
#include <string>
#include <vector>

namespace hemiola {

struct CycleDef {
	std::string name;    // short name shown on the display
	std::string family;  // "Meter", "Hindustani" or "Carnatic"
	int beats;           // total beats in the cycle
	std::vector<int> sections;  // section lengths in beats, summing to beats
	std::string source;  // where the definition was checked
};

inline int sumOf(const std::vector<int>& v) {
	int s = 0;
	for (size_t i = 0; i < v.size(); i++)
		s += v[i];
	return s;
}

inline void addCycle(std::vector<CycleDef>& t, const std::string& name, const std::string& family,
                     const std::vector<int>& sections, const std::string& source) {
	CycleDef d;
	d.name = name;
	d.family = family;
	d.sections = sections;
	d.beats = sumOf(sections);
	d.source = source;
	t.push_back(d);
}

inline std::vector<CycleDef> buildCycleTable() {
	std::vector<CycleDef> t;

	// Plain meters. The grouping is a common convention, no tradition is claimed.
	const std::string meterSrc = "Arithmetic grouping, no tradition claimed";
	addCycle(t, "2", "Meter", {2}, meterSrc);
	addCycle(t, "3", "Meter", {3}, meterSrc);
	addCycle(t, "4", "Meter", {4}, meterSrc);
	addCycle(t, "5 (3+2)", "Meter", {3, 2}, meterSrc);
	addCycle(t, "5 (2+3)", "Meter", {2, 3}, meterSrc);
	addCycle(t, "6 (3+3)", "Meter", {3, 3}, meterSrc);
	addCycle(t, "6 (2+2+2)", "Meter", {2, 2, 2}, meterSrc);
	addCycle(t, "7 (2+2+3)", "Meter", {2, 2, 3}, meterSrc);
	addCycle(t, "7 (3+2+2)", "Meter", {3, 2, 2}, meterSrc);
	addCycle(t, "8 (4+4)", "Meter", {4, 4}, meterSrc);
	addCycle(t, "9 (3+3+3)", "Meter", {3, 3, 3}, meterSrc);
	addCycle(t, "11 (3+3+3+2)", "Meter", {3, 3, 3, 2}, meterSrc);
	addCycle(t, "12 (3+3+3+3)", "Meter", {3, 3, 3, 3}, meterSrc);

	// Hindustani talas.
	const std::string hSrc = "ancient-future.com/theka.html; en.wikipedia.org/wiki/Vibhag";
	addCycle(t, "Tintal", "Hindustani", {4, 4, 4, 4}, hSrc);
	addCycle(t, "Keherwa", "Hindustani", {4, 4}, hSrc);
	addCycle(t, "Dadra", "Hindustani", {3, 3}, hSrc);
	addCycle(t, "Rupak", "Hindustani", {3, 2, 2}, hSrc);
	addCycle(t, "Jhaptal", "Hindustani", {2, 3, 2, 3}, hSrc);
	addCycle(t, "Ektal", "Hindustani", {2, 2, 2, 2, 2, 2}, hSrc);
	addCycle(t, "Dhamar", "Hindustani", {5, 2, 3, 4}, hSrc);

	// Carnatic suladi sapta talas: 7 talas x 5 jatis = 35 cycles.
	// Anga codes: L = laghu (jati beats), D = drutam (2 beats), U = anudrutam (1 beat).
	struct TalaRule { const char* name; const char* angas; };
	const TalaRule rules[7] = {
		{"Dhruva", "LDLL"}, {"Matya", "LDL"}, {"Rupaka", "DL"}, {"Jhampa", "LUD"},
		{"Triputa", "LDD"}, {"Ata", "LLDD"}, {"Eka", "L"}
	};
	const int jatis[5] = {3, 4, 5, 7, 9};
	const std::string cSrc = "karnatik.com/ctaala.shtml; acharyanet.com (anga rules)";
	for (int r = 0; r < 7; r++) {
		for (int j = 0; j < 5; j++) {
			std::vector<int> sec;
			for (const char* c = rules[r].angas; *c; c++) {
				if (*c == 'L') sec.push_back(jatis[j]);
				else if (*c == 'D') sec.push_back(2);
				else sec.push_back(1);
			}
			std::string name = std::string(rules[r].name) + " " + std::to_string(jatis[j]);
			// Triputa with a 4-beat laghu is the cycle widely known as Adi tala (8 beats, 4+2+2).
			if (r == 4 && jatis[j] == 4)
				name = "Adi";
			addCycle(t, name, "Carnatic", sec, cSrc);
		}
	}
	return t;
}

inline const std::vector<CycleDef>& cycleTable() {
	static const std::vector<CycleDef> table = buildCycleTable();
	return table;
}

inline int cycleCount() {
	return (int) cycleTable().size();
}

inline int findCycle(const std::string& name) {
	const std::vector<CycleDef>& t = cycleTable();
	for (size_t i = 0; i < t.size(); i++)
		if (t[i].name == name)
			return (int) i;
	return 0;
}

inline std::string sectionsText(const CycleDef& d) {
	std::string s;
	for (size_t i = 0; i < d.sections.size(); i++) {
		if (i) s += "+";
		s += std::to_string(d.sections[i]);
	}
	return s;
}

}  // namespace hemiola
