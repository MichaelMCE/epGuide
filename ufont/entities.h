


#ifndef _ENTITIES_H_
#define _ENTITIES_H_


#if ENABLE_CHARREFS

typedef struct __attribute__((packed)){
	const char *entity;
	uint16_t len;			// length of the entity [name] including  ';'
	uint16_t encoding;		// (unicode code point)
}_html_entity_t;

// list ordered by string length then subordered by usage probability
// list is used by isCharRef() in libuf.c
// see charsrefs.c
static const _html_entity_t entities[] = {
	{"lt;", 3, 60},			// < less-than sign 
	{"gt;", 3, 62},			// > greater-than sign 
	{"le;", 3, 8804},		// = less-than or equal to 
	{"ge;", 3, 8805},		// = greater-than or equal to 
	{"ne;", 3, 8800},		// ? not equal to 
	{"or;", 3, 8744},		// ? logical or 
	{"ni;", 3, 8715},		// ? contains as member 
	{"Mu;", 3, 924},		// ? greek capital letter mu 
	{"Nu;", 3, 925},		// ? greek capital letter nu 
	{"Xi;", 3, 926},		// ? greek capital letter xi 
	{"Pi;", 3, 928},		// ? greek capital letter pi 
	{"mu;", 3, 956},		// Ê greek small letter mu 
	{"nu;", 3, 957},		// ? greek small letter nu 
	{"xi;", 3, 958},		// ? greek small letter xi 
	{"pi;", 3, 960},		// p greek small letter pi 
	{"amp;", 4, 38},		// &  ampersand 
	{"yen;", 4, 165},		// æ yen sign 
	{"rho;", 4, 961},		// ? greek small letter rho 
	{"tau;", 4, 964},		// t greek small letter tau 
	{"phi;", 4, 966},		// f greek small letter phi 
	{"chi;", 4, 967},		// ? greek small letter chi 
	{"psi;", 4, 968},		// ? greek small letter psi 
	{"eta;", 4, 951},		// ? greek small letter eta 
	{"piv;", 4, 982},		// ? greek pi symbol
	{"Rho;", 4, 929},		// ? greek capital letter rho 
	{"uml;", 4, 168},		// ˘ diaeresis 
	{"not;", 4, 172},		// ™ not sign 
	{"shy;", 4, 173},		//  soft hyphen 
	{"reg;", 4, 174},		// © registered sign 
	{"deg;", 4, 176},		// ¯ degree sign 
	{"Eta;", 4, 919},		// ? greek capital letter eta 
	{"Tau;", 4, 932},		// ? greek capital letter tau 
	{"Phi;", 4, 934},		// F greek capital letter phi 
	{"Chi;", 4, 935},		// ? greek capital letter chi 
	{"Psi;", 4, 936},		// ? greek capital letter psi 
	{"and;", 4, 8743},		// ? logical and 
	{"zwj;", 4, 8205},		// ? zero width joiner 
	{"lrm;", 4, 8206},		// ? left-to-right mark 
	{"rlm;", 4, 8207},		// ? right-to-left mark 
	{"ang;", 4, 8736},		// ? angle 
	{"cap;", 4, 8745},		// n intersection 
	{"cup;", 4, 8746},		// ? union 
	{"int;", 4, 8747},		// ? integral 
	{"sim;", 4, 8764},		// ~ tilde operator 
	{"sub;", 4, 8834},		// ? subset of 
	{"sup;", 4, 8835},		// ? superset of 
	{"sum;", 4, 8721},		// S n-ary sumation 
	{"loz;", 4, 9674},		// ? lozenge 
	{"ETH;", 4, 208},		// — latin capital letter ETH 
	{"eth;", 4, 240},		// – latin small letter eth 
	{"quot;", 5, 34},		// "  quotation mark 
	{"euro;", 5, 8364},		// ? euro sign 
	{"nbsp;", 5, 160},		//   non-breaking space 
	{"bull;", 5, 8226},		//  bullet 
	{"apos;", 5, 39},		// ' apos
	{"circ;", 5, 710},		// ^ modifier letter circumflex accent 
	{"ensp;", 5, 8194},		//   en space 
	{"emsp;", 5, 8195},		//   em space 
	{"zwnj;", 5, 8204},		// ? zero width non-joiner 
	{"cent;", 5, 162},		// Ω cent sign 
	{"real;", 5, 8476},		// R blackletter capital R 
	{"larr;", 5, 8592},		//  leftwards arrow 
	{"uarr;", 5, 8593},		//  upwards arrow 
	{"rarr;", 5, 8594},		//  rightwards arrow 
	{"darr;", 5, 8595},		//  downwards arrow 
	{"harr;", 5, 8596},		//  left right arrow 
	{"lArr;", 5, 8656},		// ? leftwards double arrow 
	{"uArr;", 5, 8657},		// ? upwards double arrow 
	{"rArr;", 5, 8658},		// ? rightwards double arrow 
	{"dArr;", 5, 8659},		// ? downwards double arrow 
	{"hArr;", 5, 8660},		// ? left right double arrow 
	{"isin;", 5, 8712},		// ? element of 
	{"part;", 5, 8706},		// ? partial differential 
	{"prod;", 5, 8719},		// ? n-ary product 
	{"prop;", 5, 8733},		// ? proportional to 
	{"cong;", 5, 8773},		// ? approximately equal to 
	{"nsub;", 5, 8836},		// ? not a subset of 
	{"sube;", 5, 8838},		// ? subset of or equal to 
	{"supe;", 5, 8839},		// ? superset of or equal to 
	{"perp;", 5, 8869},		// ? up tack 
	{"sdot;", 5, 8901},		// ˙ dot operator 
	{"lang;", 5, 9001},		// < left-pointing angle bracket 
	{"rang;", 5, 9002},		// > right-pointing angle bracket 
	{"sect;", 5, 167},		// ı section sign 
	{"copy;", 5, 169},		// ∏ copyright sign 
	{"ordf;", 5, 170},		// ¶ feminine ordinal indicator 
	{"macr;", 5, 175},		// Ó macron 
	{"sup2;", 5, 178},		// ˝ superscript two 
	{"sup3;", 5, 179},		// ¸ superscript three 
	{"para;", 5, 182},		// Ù pilcrow sign 
	{"sup1;", 5, 185},		// ˚ superscript one 
	{"ordm;", 5, 186},		// ß masculine ordinal indicator 
	{"fnof;", 5, 402},		// ü latin small f with hook 
	{"Beta;", 5, 914},		// ? greek capital letter beta 
	{"Zeta;", 5, 918},		// ? greek capital letter zeta 
	{"Iota;", 5, 921},		// ? greek capital letter iota 
	{"beta;", 5, 946},		// · greek small letter beta 
	{"zeta;", 5, 950},		// ? greek small letter zeta 
	{"iota;", 5, 953},		// ? greek small letter iota 
	{"Yuml;", 5, 376},		// Y latin capital letter Y with diaeresis 
	{"Auml;", 5, 196},		// é latin capital letter A with diaeresis 
	{"Ouml;", 5, 214},		// ô latin capital letter O with diaeresis 
	{"Euml;", 5, 203},		// ” latin capital letter E with diaeresis 
	{"Iuml;", 5, 207},		// ÿ latin capital letter I with diaeresis 
	{"Uuml;", 5, 220},		// ö latin capital letter U with diaeresis 
	{"auml;", 5, 228},		// Ñ latin small letter a with diaeresis 
	{"euml;", 5, 235},		// â latin small letter e with diaeresis 
	{"iuml;", 5, 239},		// ã latin small letter i with diaeresis 
	{"ouml;", 5, 246},		// î latin small letter o with diaeresis 
	{"uuml;", 5, 252},		// Å latin small letter u with diaeresis 
	{"yuml;", 5, 255},		// ò latin small letter y with diaeresis 
	{"ucirc;", 6, 251},		// ñ latin small letter u with circumflex 
	{"thorn;", 6, 254},		// Á latin small letter thorn 
	{"sigma;", 6, 963},		// s greek small letter sigma 
	{"theta;", 6, 952},		// ? greek small letter theta 
	{"kappa;", 6, 954},		// ? greek small letter kappa 
	{"omega;", 6, 969},		// ? greek small letter omega 
	{"upsih;", 6, 978},		// ? greek upsilon with hook symbol 
	{"OElig;", 6, 338},		// O latin capital ligature OE 
	{"oelig;", 6, 339},		// o latin small ligature oe 
	{"Acirc;", 6, 194},		// ∂ latin capital letter A with circumflex 
	{"Aring;", 6, 197},		// è latin capital letter A with ring above 
	{"AElig;", 6, 198},		// í latin capital letter AE 
	{"Alpha;", 6, 913},		// ? greek capital letter alpha 
	{"Gamma;", 6, 915},		// G greek capital letter gamma 
	{"Delta;", 6, 916},		// ? greek capital letter delta 
	{"Theta;", 6, 920},		// ù greek capital letter theta 
	{"Kappa;", 6, 922},		// ? greek capital letter kappa 
	{"Sigma;", 6, 931},		// S greek capital letter sigma 
	{"Omega;", 6, 937},		// O greek capital letter omega 
	{"alpha;", 6, 945},		// a greek small letter alpha 
	{"gamma;", 6, 947},		// ? greek small letter gamma 
	{"delta;", 6, 948},		// d greek small letter delta 
	{"iexcl;", 6, 161},		// ≠ inverted exclamation mark 
	{"diams;", 6, 9830},	//  black diamond suit 
	{"clubs;", 6, 9827},	//  black club suit 
	{"lceil;", 6, 8968},	// ? left ceiling 
	{"rceil;", 6, 8969},	// ? right ceiling 
	{"oplus;", 6, 8853},	// ? circled plus 
	{"minus;", 6, 8722},	// - minus sign 
	{"radic;", 6, 8730},	// V square root 
	{"infin;", 6, 8734},	// 8 infinity 
	{"asymp;", 6, 8776},	// ~ almost equal to 
	{"equiv;", 6, 8801},	// = identical to 
	{"exist;", 6, 8707},	// ? there exists 
	{"empty;", 6, 8709},	// ù empty set 
	{"nabla;", 6, 8711},	// ? nabla 
	{"notin;", 6, 8713},	// ? not an element of 
	{"pound;", 6, 163},		// ú pound sign 
	{"tilde;", 6, 732},		// ~ small tilde 
	{"ndash;", 6, 8211},	// - en dash 
	{"mdash;", 6, 8212},	// - em dash 
	{"lsquo;", 6, 8216},	// ' left single quotation mark 
	{"rsquo;", 6, 8217},	// ' right single quotation mark 
	{"sbquo;", 6, 8218},	// ' single low-9 quotation mark 
	{"ldquo;", 6, 8220},	// " left double quotation mark 
	{"rdquo;", 6, 8221},	// " right double quotation mark 
	{"bdquo;", 6, 8222},	// " double low-9 quotation mark 
	{"prime;", 6, 8242},	// Ô primeminutes 
	{"Prime;", 6, 8243},	// ? double prime 
	{"oline;", 6, 8254},	// ? overline 
	{"frasl;", 6, 8260},	// / fraction slash 
	{"image;", 6, 8465},	// I blackletter capital I 
	{"trade;", 6, 8482},	// T trade mark sign 
	{"crarr;", 6, 8629},	// ? downwards arrow with corner leftwards 
	{"laquo;", 6, 171},		// Æ left-pointing double angle quotation mark 
	{"times;", 6, 215},		// û multiplication sign 
	{"acute;", 6, 180},		// Ô acute accent 
	{"micro;", 6, 181},		// Ê micro sign 
	{"cedil;", 6, 184},		// ˜ cedilla 
	{"raquo;", 6, 187},		// Ø right-pointing double angle quotation mark 
	{"Ucirc;", 6, 219},		// Í latin capital letter U with circumflex 
	{"Ecirc;", 6, 202},		// “ latin capital letter E with circumflex 
	{"Icirc;", 6, 206},		// ◊ latin capital letter I with circumflex 
	{"Ocirc;", 6, 212},		// ‚ latin capital letter O with circumflex 
	{"THORN;", 6, 222},		// Ë latin capital letter THORN 
	{"szlig;", 6, 223},		// · latin small letter sharp s 
	{"acirc;", 6, 226},		// É latin small letter a with circumflex 
	{"aring;", 6, 229},		// Ü latin small letter a with ring above 
	{"aelig;", 6, 230},		// ë latin small letter ae 
	{"ecirc;", 6, 234},		// à latin small letter e with circumflex 
	{"icirc;", 6, 238},		// å latin small letter i with circumflex 
	{"ocirc;", 6, 244},		// ì latin small letter o with circumflex 
	{"ccedil;", 7, 231},	// á latin small letter c with cedilla 
	{"egrave;", 7, 232},	// ä latin small letter e with grave 
	{"eacute;", 7, 233},	// Ç latin small letter e with acute 
	{"igrave;", 7, 236},	// ç latin small letter i with grave 
	{"iacute;", 7, 237},	// ° latin small letter i with acute 
	{"ntilde;", 7, 241},	// § latin small letter n with tilde 
	{"ograve;", 7, 242},	// ï latin small letter o with grave 
	{"oacute;", 7, 243},	// ¢ latin small letter o with acute 
	{"otilde;", 7, 245},	// ‰ latin small letter o with tilde 
	{"oslash;", 7, 248},	// õ latin small letter o with stroke 
	{"ugrave;", 7, 249},	// ó latin small letter u with grave 
	{"uacute;", 7, 250},	// £ latin small letter u with acute 
	{"Igrave;", 7, 204},	// ﬁ latin capital letter I with grave 
	{"Iacute;", 7, 205},	// ÷ latin capital letter I with acute 
	{"Ntilde;", 7, 209},	// • latin capital letter N with tilde 
	{"Ograve;", 7, 210},	// „ latin capital letter O with grave 
	{"Oacute;", 7, 211},	// ‡ latin capital letter O with acute 
	{"Otilde;", 7, 213},	// Â latin capital letter O with tilde 
	{"Oslash;", 7, 216},	// ù latin capital letter O with stroke 
	{"Ugrave;", 7, 217},	// Î latin capital letter U with grave 
	{"Uacute;", 7, 218},	// È latin capital letter U with acute 
	{"Yacute;", 7, 221},	// Ì latin capital letter Y with acute 
	{"agrave;", 7, 224},	// Ö latin small letter a with grave 
	{"aacute;", 7, 225},	// † latin small letter a with acute 
	{"atilde;", 7, 227},	// ∆ latin small letter a with tilde 
	{"plusmn;", 7, 177},	// Ò plus-minus sign 
	{"divide;", 7, 247},	// ˆ division sign 
	{"middot;", 7, 183},	// ˙ middle dot 
	{"frac14;", 7, 188},	// ¨ vulgar fraction one quarter 
	{"frac12;", 7, 189},	// ´ vulgar fraction one half 
	{"frac34;", 7, 190},	// Û vulgar fraction three quarters 
	{"iquest;", 7, 191},	// ® inverted question mark 
	{"spades;", 7, 9824},	//  black spade suit 
	{"hearts;", 7, 9829},	//  black heart suit 
	{"brvbar;", 7, 166},	// › broken bar 
	{"curren;", 7, 164},	// œ currency sign 
	{"thinsp;", 7, 8201},	// ? thin space 
	{"dagger;", 7, 8224},	// ≈ dagger 
	{"Dagger;", 7, 8225},	// Œ double dagger 
	{"permil;", 7, 8240},	// % per mille sign 
	{"lsaquo;", 7, 8249},	// < single left-pointing angle quotation mark 
	{"rsaquo;", 7, 8250},	// > single right-pointing angle quotation mark 
	{"hellip;", 7, 8230},	// . horizontal ellipsis 
	{"weierp;", 7, 8472},	// P script capital P
	{"forall;", 7, 8704},	// ? for all 
	{"there4;", 7, 8756},	// ? therefore 
	{"lowast;", 7, 8727},	// * asterisk operator 
	{"otimes;", 7, 8855},	// ? circled times 
	{"vellip;", 7, 8942},	// ? 	vertical ellipsis HTML 5.0 	
	{"lfloor;", 7, 8970},	// ? left floor 
	{"rfloor;", 7, 8971},	// ? right floor 
	{"Lambda;", 7, 923},	// ? greek capital letter lambda 
	{"sigmaf;", 7, 962},	// ? greek small letter final sigma 
	{"lambda;", 7, 955},	// ? greek small letter lambda 
	{"Scaron;", 7, 352},	// S latin capital letter S with caron 
	{"scaron;", 7, 353},	// s latin small letter s with caron 
	{"Agrave;", 7, 192},	// ∑ latin capital letter A with grave 
	{"Aacute;", 7, 193},	// µ latin capital letter A with acute 
	{"Atilde;", 7, 195},	// « latin capital letter A with tilde 
	{"Ccedil;", 7, 199},	// Ä latin capital letter C with cedilla 
	{"Egrave;", 7, 200},	// ‘ latin capital letter E with grave 
	{"Eacute;", 7, 201},	// ê  latin capital letter E with acute 	
	{"yacute;", 7, 253},	// Ï latin small letter y with acute 
	{"Epsilon;", 8, 917},	// ? greek capital letter epsilon 
	{"Omicron;", 8, 927},	// ? greek capital letter omicron 
	{"Upsilon;", 8, 933},	// ? greek capital letter upsilon 
	{"epsilon;", 8, 949},	// e greek small letter epsilon 
	{"omicron;", 8, 959},	// ? greek small letter omicron 
	{"upsilon;", 8, 965},	// ? greek small letter upsilon 
	{"alefsym;", 8, 8501},	// ? alef symbol 
	{"thetasym;", 9, 977},	// ? greek small letter theta symbol 
	{"", 0,0}
};
#endif

#endif

