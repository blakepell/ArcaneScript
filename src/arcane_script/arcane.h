#define ARCANE_VERSION_STRING "0.0.1"

#define HEADER "--------------------------------------------------------------------------------\r\n"

#define IS_NULLSTR(str)      ((str) == NULL || (str)[0] == '\0')
#define IS_SET(flag, bit)    ((flag) & (bit))
#define SET_BIT(var, bit)    ((var) |= (bit))
#define REMOVE_BIT(var, bit) ((var) &= ~(bit))
#define TOGGLE_BIT(var, bit) ((var) ^= (bit))
#define UMIN(a, b)           ((a) < (b) ? (a) : (b))
#define UMAX(a, b)           ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)      ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)             ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)             ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))