

# 0b11001010 (High, CH 4) = 1000 0000 0000 0000 - 1000 0000 0000 0000 - 0000 0000 0000 0000 - 0000 0000 0000 0000
# 0b11001010 (Low,  Ch 4) = 1000 0000 0000 0000 - 0000 0000 0000 0000 - 1000 0000 0000 0000 - 0000 0000 0000 0000

# High array
high1_str = "static const uint32_t interleaveLUT1[256] = { \\\n"
for i in range(256):
	int1 = 1 if (i & 0x80 > 0) else 0
	int2 = 1 if (i & 0x40 > 0) else 0
	high1_str += "%48s %i%s\n" % (' ', (int1 << 16 + int2), ", \\" if i < 255 else "  \\")
high1_str += "%49s" % "};\n"

# High array
high2_str = "static const uint32_t interleaveLUT2[256] = { \\\n"
for i in range(256):
	int1 = 1 if (i & 0x20 > 0) else 0
	int2 = 1 if (i & 0x10 > 0) else 0
	high2_str += "%48s %i%s\n" % (' ', (int1 << 16 + int2), ", \\" if i < 255 else "  \\")
high2_str += "%49s" % "};\n"

# High array
high3_str = "static const uint32_t interleaveLUT3[256] = { \\\n"
for i in range(256):
	int1 = 1 if (i & 0x80 > 0) else 0
	int2 = 1 if (i & 0x40 > 0) else 0
	high3_str += "%48s %i%s\n" % (' ', (int1 << 16 + int2), ", \\" if i < 255 else "  \\")
high3_str += "%49s" % "};\n"

# High array
high4_str = "static const uint32_t interleaveLUT4[256] = { \\\n"
for i in range(256):
	int1 = 1 if (i & 0x20 > 0) else 0
	int2 = 1 if (i & 0x10 > 0) else 0
	high4_str += "%48s %i%s\n" % (' ', (int1 << 16 + int2), ", \\" if i < 255 else "  \\")
high4_str += "%49s" % "};\n"

print high1_str
print "\n\n\n"
print high2_str
print "\n\n\n"
print high3_str
print "\n\n\n"
print high4_str
print "\n\n\n"