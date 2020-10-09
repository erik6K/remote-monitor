#ifndef _SIGNAL_H
#define _SIGNAL_H

#include "Adafruit_ZeroFFT.h"

q15_t signal[] =    {1977, 1963, 2057, 2137, 2151, 2102, 2001, 1931, 1962, 2054, 2125, 2129,
                     2047, 1949, 1913, 1962, 2066, 2113, 2082, 1994, 1909, 1894, 1985, 2064,
                     2090, 2052, 1949, 1872, 1903, 1990, 2062, 2080, 2003, 1902, 1865, 1909,
                     2007, 2065, 2046, 1968, 1872, 1857, 1937, 2023, 2060, 2024, 1926, 1847,
                     1876, 1957, 2031, 2064, 1993, 1888, 1852, 1890, 1976, 2060, 2049, 1961,
                     1887, 1855, 1917, 2015, 2065, 2036, 1939, 1865, 1882, 1953, 2044, 2085,
                     2017, 1921, 1880, 1902, 1994, 2082, 2082, 1999, 1938, 1887, 1938, 2055,
                     2107, 2073, 1998, 1919, 1909, 2001, 2093, 2122, 2085, 1989, 1921, 1959,
                     2048, 2128, 2149, 2077, 1979, 1956, 2003, 2100, 2172, 2155, 2072, 1996,
                     1983, 2058, 2155, 2203, 2164, 2069, 2000, 2027, 2114, 2206, 2228, 2165,
                     2073, 2034, 2071, 2183, 2249, 2243, 2173, 2071, 2059, 2125, 2235, 2282,
                     2261, 2162, 2091, 2110, 2178, 2283, 2306, 2259, 2158, 2106, 2147, 2238,
                     2332, 2315, 2255, 2155, 2140, 2192, 2257, 2356, 2325, 2245, 2153, 2165,
                     2237, 2348, 2367, 2329, 2229, 2170, 2200, 2281, 2377, 2380, 2323, 2211,
                     2181, 2236, 2331, 2397, 2377, 2294, 2208, 2199, 2271, 2366, 2410, 2363,
                     2269, 2201, 2218, 2304, 2391, 2408, 2341, 2243, 2200, 2241, 2333, 2404,
                     2393, 2306, 2220, 2197, 2262, 2356, 2401, 2367, 2259, 2205, 2200, 2279,
                     2374, 2388, 2329, 2233, 2178, 2216, 2311, 2367, 2365, 2285, 2192, 2162,
                     2218, 2312, 2364, 2332, 2238, 2157, 2151, 2223, 2313, 2343, 2287, 2183,
                     2124, 2143, 2228, 2294, 2325, 2236, 2128, 2091, 2145, 2226, 2294, 2266,
                     2175, 2093, 2071, 2138, 2234, 2267, 2215, 2128, 2054, 2060, 2144, 2226,
                     2232, 2161, 2065, 2016, 2053, 2145, 2211, 2196, 2106, 2014, 1992, 2049,
                     2145, 2189, 2145, 2044, 1969, 1987, 2051, 2132, 2148, 2088, 2004, 1937,
                     1968, 2062, 2124, 2123, 2042, 1945, 1916, 1973, 2063, 2117, 2083, 1985,
                     1901, 1897, 1976, 2068, 2092, 2041, 1941, 1876, 1904, 1992, 2076, 2074,
                     1999, 1901, 1866, 1910, 2009, 2071, 2048, 1956, 1872, 1865, 1936, 2029,
                     2069, 2019, 1924, 1847, 1868, 1960, 2040, 2056, 1986, 1893, 1849, 1893,
                     1988, 2059, 2046, 1959, 1867, 1852, 1932, 2016, 2076, 2028, 1935, 1865,
                     1872, 1963, 2050, 2072, 2015, 1918, 1863, 1913, 2011, 2075, 2078, 2000,
                     1911, 1889, 1951, 2041, 2109, 2085, 1993, 1916, 1920, 2001, 2096, 2135,
                     2069, 1978, 1924, 1956, 2052, 2138, 2145, 2074, 1985, 1952, 2008, 2118,
                     2170, 2150, 2067, 1991, 1980, 2065, 2163, 2204, 2160, 2067, 2006, 2030,
                     2122, 2212, 2223, 2158, 2057, 2033, 2081, 2181, 2251, 2239, 2159, 2075,
                     2061, 2137, 2233, 2283, 2247, 2154, 2080, 2105, 2190, 2285, 2299, 2252,
                     2157, 2106, 2153, 2246, 2326, 2318, 2239, 2156, 2133, 2196, 2312, 2355,
                     2319, 2246, 2162, 2156, 2256, 2344, 2365, 2324, 2226, 2161, 2205, 2297,
                     2371, 2385, 2305, 2210, 2184, 2241, 2329, 2402, 2374, 2280, 2207, 2200,
                     2272, 2374, 2405, 2360, 2263, 2198, 2223, 2307, 2388, 2413, 2331, 2232,
                     2199, 2241, 2335, 2409, 2388, 2297, 2215, 2197, 2261, 2359, 2403, 2353,
                     2273, 2193, 2199, 2300, 2374, 2383, 2329, 2228, 2167, 2221, 2304, 2368,
                     2373, 2281, 2180, 2171, 2223, 2310, 2374, 2328, 2223, 2161, 2153, 2224,
                     2325, 2342, 2276, 2189, 2121, 2144, 2240, 2310, 2300, 2226, 2130, 2089,
                     2145, 2241, 2289, 2273, 2171, 2079, 2081, 2150};

#endif