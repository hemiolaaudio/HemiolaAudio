// Hemiola Audio: nadai engine (the code still calls it Gati). Splits each incoming beat into equal parts.
// The first part lands exactly on the beat. The other parts are placed using the
// length of the previous beat. A new division only starts on the next beat.
#pragma once

namespace hemiola {

class GatiEngine {
public:
	static const int kMinDivision = 1;
	static const int kMaxDivision = 9;

	// Ask for a new division. It applies on the next beat.
	void setDivision(int n) {
		if (n < kMinDivision) n = kMinDivision;
		if (n > kMaxDivision) n = kMaxDivision;
		pending = n;
	}

	void reset() {
		elapsed = 0.0;
		prevPeriod = 0.0;
		edgeSeen = false;
		fired = kMaxDivision + 1;  // nothing fires until the next beat
		active = pending;
	}

	// Call once per sample. beatEdge is true on the sample where a beat arrives.
	// Returns true when a part fires on this sample; first is true for the on-beat part.
	bool process(bool beatEdge, bool& first) {
		first = false;
		elapsed += 1.0;
		if (beatEdge) {
			if (edgeSeen && elapsed > 1.0)
				prevPeriod = elapsed;
			edgeSeen = true;
			elapsed = 0.0;
			active = pending;
			fired = 1;
			first = true;
			return true;
		}
		if (prevPeriod > 0.0 && fired < active) {
			double due = (double) fired * prevPeriod / (double) active;
			if (elapsed >= due) {
				fired++;
				return true;
			}
		}
		return false;
	}

	int activeDivision() const { return active; }
	int pendingDivision() const { return pending; }
	bool hasTempo() const { return prevPeriod > 0.0; }

private:
	int active = 4;
	int pending = 4;
	int fired = kMaxDivision + 1;
	bool edgeSeen = false;
	double elapsed = 0.0;
	double prevPeriod = 0.0;
};

// Hemiola Audio: part divider. Counts the parts coming out of the Gati engine and passes on
// every Nth one, so a fast nadai can be followed at a slower pace that stays on the same grid.
// The first part after a change of N (and after reset) is always passed on.
class PartDivider {
public:
	static const int kMinDivide = 1;
	static const int kMaxDivide = 64;

	void setDivide(int n) {
		if (n < kMinDivide) n = kMinDivide;
		if (n > kMaxDivide) n = kMaxDivide;
		if (n != divide) {
			divide = n;
			count = 0;
		}
	}

	void reset() { count = 0; }

	// Call once per sample with true on every sample where a part fires.
	bool process(bool part) {
		if (!part)
			return false;
		bool pass = (count == 0);
		count++;
		if (count >= divide)
			count = 0;
		return pass;
	}

	int divideBy() const { return divide; }

private:
	int divide = 1;
	int count = 0;
};

}  // namespace hemiola
