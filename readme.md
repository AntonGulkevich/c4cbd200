### c4cbd200 - one more log reader

### Build Requirements:
* Windows 10 SDK: 10.0.17134.0
* Visual Studio 2017 (v141 - toolset)

### Run with parameters:
* Hint: "fullpath/filename.extension" "m\*ask?\*"
* Result2: 
  * on match: return 0 and print string
  * on mismatch: return -1 and print "none"
  * on wrong parameters: return -1 and print hint
  
### Syntax:
- [x] * zero or more characters 
- [x] ? any single character
- [x] "abc" string abc

### Passed Tests:
		// Cases with repeating character sequences.

		allOk &= Try("abc", "*a*b*b*", false);
		allOk &= Try("abc", "********a********b********b********", false);

		allOk &= Try("abcccd", "*ccd", true);
		allOk &= Try("mississipissippi", "*issip*ss*", true);
		allOk &= Try("xxxx*zzzzzzzzy*f", "xxxx*zzy*fffff", false);
		allOk &= Try("xxxx*zzzzzzzzy*f", "xxx*zzy*f", true);
		allOk &= Try("xxxxzzzzzzzzyf", "xxxx*zzy*fffff", false);
		allOk &= Try("xxxxzzzzzzzzyf", "xxxx*zzy*f", true);

		allOk &= Try("xyxyxyzyxyz", "xy*z*xyz", true);
		allOk &= Try("mississippi", "*sip*", true);
		allOk &= Try("xyxyxyxyz", "xy*xyz", true);
		allOk &= Try("mississippi", "mi*sip*", true);
		allOk &= Try("ababac", "*abac*", true);
		allOk &= Try("ababac", "*abac*", true);
		allOk &= Try("aaazz", "a*zz*", true);
		allOk &= Try("a12b12", "*12*23", false);
		allOk &= Try("a12b12", "a12b", false);
		allOk &= Try("a12b12", "*12*12*", true);

		// Additional cases where the '*' char appears in the tame string.
		allOk &= Try("*", "*", true);
		allOk &= Try("a*abab", "a*b", true);
		allOk &= Try("a*r", "a*", true);
		allOk &= Try("a*ar", "a*aar", false);

		// More double wildcard scenarios.
		allOk &= Try("XYXYXYZYXYz", "XY*Z*XYz", true);
		allOk &= Try("missisSIPpi", "*SIP*", true);
		allOk &= Try("mississipPI", "*issip*PI", true);
		allOk &= Try("xyxyxyxyz", "xy*xyz", true);
		allOk &= Try("miSsissippi", "mi*sip*", true);
		allOk &= Try("miSsissippi", "mi*Sip*", false);
		allOk &= Try("abAbac", "*Abac*", true);
		allOk &= Try("abAbac", "*Abac*", true);
		allOk &= Try("aAazz", "a*zz*", true);
		allOk &= Try("A12b12", "*12*23", false);
		allOk &= Try("a12B12", "*12*12*", true);
		allOk &= Try("oWn", "*oWn*", true);

		// Completely tame (no wildcards) cases.
		allOk &= Try("bLah", "bLah", true);
		allOk &= Try("bLah", "bLaH", false);

		// Simple mixed wildcard tests suggested by IBMer Marlin Deckert.
		allOk &= Try("a", "*?", true);
		allOk &= Try("ab", "*?", true);
		allOk &= Try("abc", "*?", true);

		// More mixed wildcard tests including coverage for false positives.
		allOk &= Try("a", "?", true);
		allOk &= Try("a", "??", false);
		allOk &= Try("ab", "?*?", true);
		allOk &= Try("ab", "*?*?*", true);
		allOk &= Try("abc", "?**?*?", true);
		allOk &= Try("abc", "?**?*&?", false);
		allOk &= Try("abcd", "?b*??", true);
		allOk &= Try("abcd", "?a*??", false);
		allOk &= Try("abcd", "?**?c?", true);
		allOk &= Try("abcd", "?**?d?", false);
		allOk &= Try("abcde", "?*b*?*d*?", true);

		// Single-end
		allOk &= Try("pNELOnialisANDdySayANDySayANDdySay", "?NELO*AND?S*y", true);
		allOk &= Try("pNELOnialisANDdySayANDySayANDdySay", "?NELO*AND?S*?y", true);
		allOk &= Try("pNELOnialisANDdySayANDySayANDdySay", "?NELO*AND?S*a?", true);

		// Single-character-match cases.
		allOk &= Try("bLaaa", "bLa?*", true);
		allOk &= Try("bLah", "bL?h", true);
		allOk &= Try("bLah", "bLa?", true);
		allOk &= Try("bLaH", "?Lah", false);
		allOk &= Try("bLaH", "?LaH", true);

		// Many-wildcard scenarios.
		allOk &= Try("aaabbaabbaab", "*aabbaa*a*", true);
		allOk &= Try("abc", "********a********b********c********", true);
		allOk &= Try("********a********b********c********", "abc", false);
		allOk &= Try("*abc*", "***a*b*c***", true);
		allOk &= Try(
			"abc*abcd*abcd*abc*abcd*abcd*abc*abcd*abc*abc*abcd",
			"abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abcd", true);
		allOk &= Try("abc*abcd*abcd*abc*abcd", "abc*abc*abc*abc*abc", false);

		allOk &= Try("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab",
    "a*a*a*a*a*a*aa*aaa*a*a*b", true);
		allOk &= Try("abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
    "*a*b*ba*ca*a*aa*aaa*fa*ga*b*", true);
		allOk &= Try("abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
    "*a*b*ba*ca*a*x*aaa*fa*ga*b*", false);
		allOk &= Try("abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
    "*a*b*ba*ca*aaaa*fa*ga*gggg*b*", false);
		allOk &= Try("abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
    "*a*b*ba*ca*aaaa*fa*ga*ggg*b*", true);
		allOk &= Try("a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*",
			"a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", true);
		allOk &= Try("aaaaaaaaaaaaaaaa",
			"*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", false);
		allOk &= Try("abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn",
    "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*", false);
		allOk &= Try("abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn",
    "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*", true);
