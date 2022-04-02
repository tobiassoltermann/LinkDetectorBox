/*
  Read truetype(.ttf) from SD and generate bitmap.

  TrueType™ Reference Manual
  https://developer.apple.com/fonts/TrueType-Reference-Manual/
*/

#define TRUETYPE_H
#include "GUI_Paint.h"
#if !defined _SPI_H_INCLUDED
#include "SPI.h"
#endif /*_SPI_H_INCLUDED*/

#if defined ESP32
#include "FS.h"
#endif /*FS_H*/

#define FLAG_ONCURVE (1 << 0)
#define FLAG_XSHORT (1 << 1)
#define FLAG_YSHORT (1 << 2)
#define FLAG_REPEAT (1 << 3)
#define FLAG_XSAME (1 << 4)
#define FLAG_YSAME (1 << 5)

#define TEXT_ALIGN_LEFT 0
#define TEXT_ALIGN_CENTER 1
#define TEXT_ALIGN_RIGHT 2

#define ROTATE_0 0
#define ROTATE_90 1
#define ROTATE_180 2
#define ROTATE_270 3

typedef struct {
  char name[5];
  uint32_t checkSum;
  uint32_t offset;
  uint32_t length;
} ttTable_t;

typedef struct {
  uint32_t version;
  uint32_t revision;
  uint32_t checkSumAdjustment;
  uint32_t magicNumber;
  uint16_t flags;
  uint16_t unitsPerEm;
  char     created[8];
  char     modified[8];
  int16_t  xMin;
  int16_t  yMin;
  int16_t  xMax;
  int16_t  yMax;
  uint16_t macStyle;
  uint16_t lowestRecPPEM;
  int16_t fontDirectionHint;
  int16_t indexToLocFormat;
  int16_t glyphDataFormat;
} ttHeadttTable_t;

typedef struct {
  uint8_t flag;
  int16_t x;
  int16_t y;
} ttPoint_t;

typedef struct {
  int16_t numberOfContours;
  int16_t xMin;
  int16_t yMin;
  int16_t xMax;
  int16_t yMax;
  uint16_t *endPtsOfContours;
  uint16_t numberOfPoints;
  ttPoint_t *points;
} ttGlyph_t;

typedef struct {
  int16_t dx;
  int16_t dy;
  uint8_t enableScale;
  uint16_t scale_x;
  uint16_t scale_y;
} ttGlyphTransformation_t;

/* currently only support format4 cmap tables */
typedef struct {
  uint16_t version;
  uint16_t numberSubtables;
} ttCmapIndex_t;

typedef struct {
  uint16_t platformId;
  uint16_t platformSpecificId;
  uint16_t offset;
} ttCmapEncoding_t;

typedef struct {
  uint16_t format;
  uint16_t length;
  uint16_t language;
  uint16_t segCountX2;
  uint16_t searchRange;
  uint16_t entrySelector;
  uint16_t rangeShift;
  uint32_t offset;
  uint32_t endCodeOffset;
  uint32_t startCodeOffset;
  uint32_t idDeltaOffset;
  uint32_t idRangeOffsetOffset;
  uint32_t glyphIndexArrayOffset;
} ttCmapFormat4_t;

/* currently only support format0 kerning tables */
typedef struct {
  uint32_t version; //The version number of the kerning table (0x00010000 for the current version).
  uint32_t nTables; //The number of subtables included in the kerning table.
} ttKernHeader_t;

typedef struct {
  uint32_t length; //The length of this subtable in bytes, including this header.
  uint16_t coverage; //Circumstances under which this table is used. See below for description.
} ttKernSubtable_t;

typedef struct {
  uint16_t nPairs; //The number of kerning pairs in this subtable.
  uint16_t searchRange; //The largest power of two less than or equal to the value of nPairs, multiplied by the size in bytes of an entry in the subtable.
  uint16_t entrySelector; //This is calculated as log2 of the largest power of two less than or equal to the value of nPairs. This value indicates how many iterations of the search loop have to be made. For example, in a list of eight items, there would be three iterations of the loop.
  uint16_t rangeShift; //The value of nPairs minus the largest power of two less than or equal to nPairs. This is multiplied by the size in bytes of an entry in the table.
} ttKernFormat0_t;

typedef struct {
  int16_t x;
  int16_t y;
} ttCoordinate_t;

typedef struct {
  uint16_t advanceWidth;
  int16_t leftSideBearing;
} ttHMetric_t;

typedef struct {
  uint16_t p1;
  uint16_t p2;
  uint8_t up;
} ttWindIntersect_t;


class truetypeClass {
  public:
    truetypeClass();

    uint8_t setTtfFile(File _file, uint8_t _checkCheckSum = 0);
    void setFramebuffer(uint16_t _framebufferWidth, uint16_t _framebufferHeight, uint16_t _framebuffer_bit, uint8_t _framebufferDirection, uint8_t *_framebuffer);
    void setCharacterSpacing(int16_t _characterSpace, uint8_t _kerning = 1);
    void setCharacterSize(uint16_t _characterSize);
    void setTextBoundary(uint16_t _start_x, uint16_t _end_x, uint16_t _end_y);
    void setTextColor(uint8_t _onLine, uint8_t _inside);
    void overrideSetPixel();
    
#define setTextColour setTextColor //to satisfy a pedantic old Australian
    void setTextRotation(uint16_t _rotation);

    uint16_t getStringWidth(const wchar_t _character[]);
    uint16_t getStringWidth(const char _character[]);
    uint16_t getStringWidth(const String _string);

    void textDraw(int16_t _x, int16_t _y, const wchar_t _character[]);
    void textDraw(int16_t _x, int16_t _y, const char _character[]);
    void textDraw(int16_t _x, int16_t _y, const String _string);

    void end();

  private:
    File file;

    uint16_t charCode;
    int16_t xMin, xMax, yMin, yMax;

    const int numTablesPos = 4;
    const int tablePos = 12;

    uint16_t numTables;
    ttTable_t *table;
    ttHeadttTable_t headTable;

    uint8_t getUInt8t();
    int16_t getInt16t();
    uint16_t getUInt16t();
    uint32_t getUInt32t();
    int16_t swap_int16(int16_t _val);
    uint16_t swap_uint16(uint16_t _val);
    uint32_t swap_uint32(uint32_t _val);

    //basic
    uint32_t calculateCheckSum(uint32_t offset, uint32_t length);
    uint32_t seekToTable(const char *name);
    int readTableDirectory(int checkCheckSum);
    void readHeadTable();
    void readCoords(char _xy, uint16_t _startPoint = 0);

    //Glyph
    ttGlyphTransformation_t glyphTransformation;
    uint32_t getGlyphOffset(uint16_t index);
    uint16_t codeToGlyphId(uint16_t code);
    uint8_t readSimpleGlyph(uint8_t _addGlyph = 0);
    uint8_t readCompoundGlyph();

    //cmap. maps character codes to glyph indices
    ttCmapIndex_t cmapIndex;
    ttCmapEncoding_t *cmapEncoding;
    ttCmapFormat4_t cmapFormat4;
    uint8_t readCmapFormat4();
    uint8_t readCmap();

    //hmtx. metric information for the horizontal layout each of the glyphs
    uint32_t hmtxTablePos = 0;
    uint8_t readHMetric();
    ttHMetric_t getHMetric(uint16_t _code);

    //kerning.
    ttKernHeader_t kernHeader;
    ttKernSubtable_t kernSubtable;
    ttKernFormat0_t kernFormat0;
    uint32_t kernTablePos = 0;
    uint8_t readKern();
    int16_t getKerning(uint16_t _left_glyph, uint16_t _right_glyph);

    //generate points
    ttCoordinate_t *points;
    uint16_t numPoints;
    uint16_t *beginPoints;
    uint16_t numBeginPoints;
    uint16_t *endPoints;
    uint16_t numEndPoints;

    //glyf
    ttGlyph_t glyph;
    ttWindIntersect_t *pointsToFill;
    void generateOutline(int16_t _x, int16_t _y, uint16_t _width);
    void freePointsAll();
    bool isInside(int16_t _x, int16_t _y);
    void fillGlyph(uint16_t _x_min, uint16_t _y_min, uint16_t _width);
    uint8_t readGlyph(uint16_t code, uint8_t _justSize = 0);
    void freeGlyph();

    void addLine(int16_t _x0, int16_t _y0, int16_t _x1, int16_t _y1);
    void addPoint(int16_t _x, int16_t _y);
    void freePoints();
    void addBeginPoint(uint16_t _bp);
    void freeBeginPoints();
    void addEndPoint(uint16_t _ep);
    void freeEndPoints();
    int32_t isLeft(ttCoordinate_t *_p0, ttCoordinate_t *_p1, ttCoordinate_t *_point);

    //write user framebuffer
    uint16_t characterSize = 20;
    uint8_t kerningOn = 1;
    int16_t characterSpace = 0;
    int16_t start_x = 10;
    int16_t end_x = 500;
    int16_t end_y = 500;
    uint16_t displayWidth = 480;
    uint16_t displayHeight = 400;
    uint16_t displayWidthFrame = 480;
    uint16_t framebufferBit = 8;
    uint8_t framebufferDirection = 0;
    uint8_t stringRotation = 0x00;
    uint8_t colorLine = 0x00;
    uint8_t colorInside = 0x00;
    uint8_t *userFrameBuffer;
    void addPixel(int16_t _x, int16_t _y, uint8_t _colorCode);
    void stringToWchar(String _string, wchar_t _charctor[]);

    uint8_t GetU8ByteCount(char _ch);
    bool IsU8LaterByte(char _ch);
};
