/* $Id: emf.h 306 2015-01-29 18:45:54Z pjohnson $
    --------------------------------------------------------------------------
    Add-on package to R to produce EMF graphics output (for import as
    a high-quality vector graphic into Microsoft Office or OpenOffice).


    Copyright (C) 2011 Philip Johnson

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


    Note this header file is C++ (R policy requires that all headers
    end with .h).
    --------------------------------------------------------------------------
*/

#include <stdexcept>
#include <string>
#include <vector>

#include "emf.h"

// structs for EMF+
namespace EMFPLUS {
    using EMF::TUInt4;
    using EMF::TUInt2;
    using EMF::TUInt1;
    using EMF::TFloat4;

    enum ERecordType {
        eRcdHeader = 0x4001,
        eRcdEndOfFile = 0x4002,
        eRcdGetDC = 0x4004,
        eRcdObject = 0x4008,
        eRcdDrawRects = 0x400B,
        eRcdFillPolygon = 0x400C,
        eRcdDrawLines = 0x400D,
        eRcdFillEllipse = 0x400E,
        eRcdDrawEllipse = 0x400F,
        eRcdFillPath = 0x4014,
        eRcdDrawPath = 0x4015,
        eRcdDrawImage = 0x401A,
        eRcdDrawString = 0x401C,
        eRcdSetAntiAliasMode = 0x401E,
        eRcdSetTextRenderingHint = 0x401F,
        eRcdSetInterpolationMode = 0x4021,
        eRcdSetPixelOffsetMode = 0x4022,
        eRcdSetCompositingQuality = 0x4024,
        eRcdSetWorldTransform = 0x402A,
        eRcdResetWorldTransform = 0x402B,
        eRcdMultiplyWorldTransform = 0x402C,
        eRcdTranslateWorldTransform = 0x402D,
        eRcdSetPageTransform = 0x4030,
        eRcdSetClipRect = 0x4032
    };

    enum EObjectType {
        eTypeBrush = 1,
        eTypePen = 2,
        eTypePath = 3,
        eTypeImage = 5,
        eTypeFont = 6,
        eTypeStringFormat = 7
    };

    enum EUnitType {
        eUnitWorld = 0,
        eUnitDisplay = 1,
        eUnitPixel = 2,
        eUnitPoint = 3,
        eUnitInch = 4,
        eUnitDocument = 5,
        eUnitMillimeter = 6
    };

    enum ELineCapType {
        eCapFlat =  0,
        eCapSquare = 1,
        eCapRound = 2
    };
    enum ELineJoinType {
        eJoinMiter =  0,
        eJoinBevel = 1,
        eJoinRound = 2
    };
    enum ELineStyleType {
        eLineSolid =  0,
        eLineDash = 1,
        eLineDot = 2,
        eLineDashDot = 3,
        eLineDashDotDot = 4,
        eLineCustom = 5
    };
    enum EPenDataFlags {
        ePenStartCap = 0x02,
        ePenEndCap = 0x04,
        ePenJoin = 0x08,
        ePenMiterLimit = 0x10,
        ePenLineStyle = 0x20,
        ePenDashedLineCap = 0x40,
        ePenDashedLine = 0x100
    };
    enum EFontStyle {
        eFontBold = 1,
        eFontItalic = 2
    };

    enum EStringAlign {
        eStrAlignNear = 0,
        eStrAlignCenter = 1,
        eStrAlignFar = 2
    };

    enum ETextRenderingHint {
        eTRHAntiAliasGridFit = 3,
        eTRHClearTypeGridFit = 5
    };

    enum EAntiAliasMode {
        eAntiAliasModeHighQuality = 2,
        eAntiAliasModeNone = 3
    };

    enum EInterpolationMode {
        eInterpolationModeBilinear = 3,
        eInterpolationModeNearestNeighbor = 5,
        eInterpolationModeHighQualityBilinear = 6,
        eInterpolationModeHighQualityBicubic = 7
    };

    enum EPixelOffsetMode {
        ePixelOffsetModeDefault = 0,
        ePixelOffsetModeHighSpeed = 1,
        ePixelOffsetModeHighQuality = 2,
        ePixelOffsetModeNone = 3,
        ePixelOffsetModeHalf = 4
    };
    
    enum ECompositingQuality {
        eCompositingQualityDefault = 1,
        eCompositingQualityHighSpeed = 2,
        eCompositingQualityHighQuality = 3
    };
    
    enum ECombineMode {
        eCombineModeReplace = 0,
        eCombineModeIntersect = 1,
        eCombineModeUnion = 2,
        eCombineModeXOR = 3,
        eCombineModeExclude = 4,
        eCombineModeComplement = 5
    };

    enum EBrushType {
        eBrushTypeSolidColor = 0,
        eBrushTypeHatchFill = 1,
        eBrushTypeTextureFill = 2,
        eBrushTypePathGradient = 3,
        eBrushTypeLinearGradient = 4
    };

    enum EWrapMode {
        eWrapModeTile = 0,
        eWrapModeTileFlipX = 1,
        eWrapModeTileFlipY = 2,
        eWrapModeTileFlipXY = 3,
        eWrapModeClamp = 4
    };

    enum EPathPointType {
        ePathPointTypeStart = 0,
        ePathPointTypeLine = 1,
        ePathPointTypeBezier = 3
    };

    // ------------------------------------------------------------------------
    // EMF Objects used repeatedly
    const TUInt4 kVersion = 0xDBC01002; //specifies EMF+ and GDI+ version 1.1
    const unsigned int kMaxObjTableSize = 64; //max entries in object table

    struct SPointF {
        double x, y;
        SPointF(void) { x = y = 0; }
        SPointF(double xx, double yy) : x(xx), y(yy) {}
        friend std::string& operator<< (std::string &o, const SPointF &d) {
            return o << TFloat4(d.x) << TFloat4(d.y);
        }
    };

    struct SRectF {
        double x, y, w, h;
        SRectF(void) { x = y = w = h = 0; }
        friend std::string& operator<< (std::string &o, const SRectF &d) {
            return o << TFloat4(d.x) << TFloat4(d.y)
                     << TFloat4(d.w) << TFloat4(d.h);
        }
    };

    struct SColorRef {
        TUInt1 red, green, blue, alpha;
        void Set(unsigned char r, unsigned char g, unsigned char b,
                 unsigned char a) {
            red = r;  green = g;  blue = b; alpha = a;
        }
        SColorRef(void) {}
        SColorRef(unsigned int c) {
            Set(R_RED(c), R_GREEN(c), R_BLUE(c), R_ALPHA(c));
        }
        SColorRef& operator= (unsigned int c) {
            Set(R_RED(c), R_GREEN(c), R_BLUE(c), R_ALPHA(c));
            return *this;
        }
        friend std::string& operator<< (std::string &o, const SColorRef &d) {
            return o << d.blue << d.green << d.red << d.alpha;
        }
    };

    // ------------------------------------------------------------------------
    // EMF Records + EMF objects used in only one record begin here

    struct SRecord {
        ERecordType iType;
        unsigned short iFlags;
        TUInt4 nSize;
        TUInt4 nDataSize;
        SRecord(ERecordType t) : iType(t), iFlags(0), nSize(0), nDataSize(0) {}
        virtual std::string& Serialize(std::string &o) const {
            return o << TUInt2(iType) << TUInt2(iFlags) << nSize << nDataSize;
        }
        void Write(EMF::ofstream &o) {
            if (!o.inEMFplus) { //write encapsulating EMF record
                EMF::SPlusRecord emr;
                emr.Write(o);
                o.emfPlusStartPos = o.tellp();
                o.inEMFplus = true;
            }
            std::string buff; Serialize(buff);
            buff.resize(((buff.size() + 3)/4)*4, '\0'); //add padding
            std::string dataSize; dataSize << TUInt4(buff.size()-12);
            std::string finalSize; finalSize << TUInt4(buff.size());
            buff.replace(4,4, finalSize);
            buff.replace(8,4, dataSize);
            o.write(buff.data(), buff.size());

            // update the size of the encapsulating EMF record
            std::streampos currPos = o.tellp();
            // back up to Size field
            o.seekp(o.emfPlusStartPos - (std::streampos)12);
            buff.clear();
            buff << TUInt4((int)(currPos - o.emfPlusStartPos) + 16)
                 << TUInt4((int)(currPos - o.emfPlusStartPos) + 4);
            o.write(buff.data(), buff.size());
            o.seekp(currPos);

            if (iType == eRcdEndOfFile) {
                o.inEMFplus = false;
            }
        }
    };

    struct SHeader : SRecord {
        TUInt4 version;
        TUInt4 plusFlags;
        TUInt4 dpiX;
        TUInt4 dpiY;
        SHeader(void) : SRecord(eRcdHeader) {}
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << kVersion
                                         << plusFlags << dpiX << dpiY;
        }
    };
    struct SGetDC : SRecord {
        SGetDC(void) : SRecord(eRcdGetDC) {}
    };
    void GetDC(EMF::ofstream &o) { //forward declared in emf.h
        SGetDC emr; emr.Write(o);
    }

    struct SEndOfFile : SRecord {
        SEndOfFile(void) : SRecord(eRcdEndOfFile) {}
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o);
        }
    };

    struct SSetPixelOffsetMode : SRecord {
        SSetPixelOffsetMode(EPixelOffsetMode m) :
        SRecord(eRcdSetPixelOffsetMode) {
            iFlags = m;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o);
        }
    };

    struct SSetAntiAliasMode : SRecord {
        SSetAntiAliasMode(bool aa) : SRecord(eRcdSetAntiAliasMode) {
            iFlags = aa ? ((eAntiAliasModeHighQuality << 1) | 1):
                (eAntiAliasModeNone << 1);
        }
    };

    struct SSetTextRenderingHint : SRecord {
        SSetTextRenderingHint(ETextRenderingHint hint) : SRecord
            (eRcdSetTextRenderingHint) {
            iFlags = hint;
        }
    };

    struct SSetInterpolationMode : SRecord {
        SSetInterpolationMode(EInterpolationMode m) :
        SRecord(eRcdSetInterpolationMode) {
            iFlags = m;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o);
        }
    };

    struct SSetCompositingQuality : SRecord {
        SSetCompositingQuality(ECompositingQuality m) :
        SRecord(eRcdSetCompositingQuality) {
            iFlags = m;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o);
        }
    };

    struct SSetWorldTransform : SRecord {
        TFloat4 m_Matrix[6];
        SSetWorldTransform(double m11, double m12, double m21, double m22,
                            double dx, double dy) :
            SRecord(eRcdSetWorldTransform) {
            m_Matrix[0] = m11;
            m_Matrix[1] = m12;
            m_Matrix[2] = m21;
            m_Matrix[3] = m22;
            m_Matrix[4] = dx;
            m_Matrix[5] = dy;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << m_Matrix[0] << m_Matrix[1] <<
                m_Matrix[2] << m_Matrix[3] << m_Matrix[4] << m_Matrix[5];
        }
    };
    
    struct SResetWorldTransform : SRecord {
        SResetWorldTransform(void) : SRecord(eRcdResetWorldTransform) {}
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o);
        }
    };

    struct SMultiplyWorldTransform : SRecord {
        TFloat4 m_Matrix[6];
        SMultiplyWorldTransform(double m11, double m12, double m21, double m22,
                                double dx, double dy) :
        SRecord(eRcdMultiplyWorldTransform) {
            m_Matrix[0] = m11;
            m_Matrix[1] = m12;
            m_Matrix[2] = m21;
            m_Matrix[3] = m22;
            m_Matrix[4] = dx;
            m_Matrix[5] = dy;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << m_Matrix[0] << m_Matrix[1] <<
                m_Matrix[2] << m_Matrix[3] << m_Matrix[4] << m_Matrix[5];
        }
    };

    struct STranslateWorldTransform : SRecord {
        TFloat4 m_d[2];
        STranslateWorldTransform(double dx, double dy) :
            SRecord(eRcdTranslateWorldTransform) {
            m_d[0] = dx; m_d[1] = dy;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << m_d[0] << m_d[1];
        }
    };

    struct SSetPageTransform : SRecord {
        TFloat4 m_Scale;
        SSetPageTransform(EUnitType u, double s) :
            SRecord(eRcdSetPageTransform) { iFlags = u; m_Scale = s;}
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << m_Scale;
        }
    };

    struct SSetClipRect : SRecord {
        SRectF m_Rect;
        SSetClipRect(ECombineMode cm,
                     double x0, double y0, double x1, double y1) :
        SRecord(eRcdSetClipRect) { iFlags = cm << 8;
            m_Rect.x = std::min(x0, x1);
            m_Rect.y = std::min(y0, y1);
            m_Rect.w = abs(x1-x0);
            m_Rect.h = abs(y1-y0);}
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << m_Rect;
        }
    };

    struct SObject : SRecord {
        EObjectType type;
        SObject(EObjectType t) : SRecord(eRcdObject), type(t) {}
        virtual ~SObject(void) {}
        void SetObjId(unsigned char id) {
            iFlags = ((unsigned int)type << 8) | id;
        }
        unsigned char GetObjId(void) const {
            return iFlags & 0xFF;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o);
        }
    };

    struct SBrush : SObject {
        EBrushType brushType;
        SColorRef color;
        EWrapMode wrapMode;
        SRectF gradCoords;
        struct SBlend {
            double pos;
            SColorRef col;
            friend bool operator< (const SBlend &b1, const SBlend &b2) {
                return memcmp(&b1, &b2, sizeof(SBlend)) < 0;
            }
        };
        std::vector<SBlend> blendVector;
        SBrush(unsigned int c) : SObject(eTypeBrush),
                                 brushType(eBrushTypeSolidColor),
                                 color(c), wrapMode(eWrapModeTile) {}
        SBrush(EBrushType bt) : SObject(eTypeBrush), brushType(bt) {}
        std::string& Serialize(std::string &o) const {
            SObject::Serialize(o) << kVersion << TUInt4(brushType);
            switch(brushType) {
            case eBrushTypeSolidColor:
                return o << color;
            case eBrushTypeLinearGradient:
                o << TUInt4(0x4)//preset colors flag
                  << TUInt4(wrapMode) << gradCoords
                  << color << color //start&end colors not used
                  << TUInt4(0) << TUInt4(0) //reserved padding
                  << TUInt4(blendVector.size());
                for (unsigned int i = 0;  i < blendVector.size();  ++i) {
                    o << TFloat4(blendVector[i].pos);
                }
                for (unsigned int i = 0;  i < blendVector.size();  ++i) {
                    o << blendVector[i].col;
                }
                return o;
            default:
                throw std::logic_error("unhandled brush type");
            }
        }
        friend bool operator< (const SBrush& b1, const SBrush& b2) {
            int cmp = memcmp(&b1.brushType, &b2.brushType, (char*)(&b1.blendVector) - (char*)&b1.brushType);
            return (cmp < 0 || (cmp == 0  &&  b1.blendVector < b2.blendVector));
        }
    };

    struct SPenData {
        TFloat4 width;
        //"optional" data below (depends on flags)
        TUInt4 startCap;
        TUInt4 endCap;
        TUInt4 join;
        TFloat4 miterLimit;
        TUInt4 lineStyle;
        TUInt4 dashedCap;
        std::vector<double> dashedLineData; //for custom line style
        std::string& Serialize(std::string &o) const {
            o << TUInt4(ePenStartCap | ePenEndCap | ePenJoin | ePenMiterLimit |
                        ePenLineStyle | ePenDashedLineCap |
                        (!dashedLineData.empty() ? ePenDashedLine : 0))
              << TUInt4(eUnitWorld)
              << width << startCap << endCap << join << miterLimit
              << lineStyle << dashedCap;
            if (!dashedLineData.empty()) {
                o << TUInt4(dashedLineData.size());
                for (unsigned int i = 0;  i < dashedLineData.size();  ++i) {
                    o << TFloat4(dashedLineData[i]);
                }
            }
            return o;
        }
    };
    struct SPen : SObject {
        SPenData pen;
        SColorRef brush;
        SPen(unsigned int col, double lwd, unsigned int lty,
             unsigned int lend, unsigned int ljoin, unsigned int lmitre,
             double ps2dev, bool useUserLty);
        std::string& Serialize(std::string &o) const {
            SObject::Serialize(o) << kVersion << TUInt4(0); //always 0
            pen.Serialize(o);
            o << kVersion << TUInt4(0) /*SOLID*/ << brush;
            return o;
        }
    };

    struct SFont : SObject {
        double m_emSize;
        unsigned int m_Style;
        std::string m_FamilyUTF16; //UTF-16 characters
        SFont(unsigned char face, double size, const std::string &familyUTF16) :
            SObject(eTypeFont) {
            m_Style =
                ((face == 2  ||  face == 4) ? eFontBold : 0) |
                ((face == 3  ||  face == 4) ? eFontItalic : 0);
            m_emSize = size;
            m_FamilyUTF16 = familyUTF16;
        }
        std::string& Serialize(std::string &o) const {
            SObject::Serialize(o);
            o << kVersion << TFloat4(m_emSize) << TUInt4(eUnitWorld)
              << TUInt4(m_Style) << TUInt4(0)
              << TUInt4(m_FamilyUTF16.length()/2);
            o.append(m_FamilyUTF16);
            return o;
        }
    };

    struct SStringFormat : SObject {
        EStringAlign m_Horiz;
        EStringAlign m_Vert;

        SStringFormat(EStringAlign h, EStringAlign v) :
            SObject(eTypeStringFormat) { m_Horiz=h; m_Vert=v;}
        std::string& Serialize(std::string &o) const {
            SObject::Serialize(o);
            o << kVersion
              << TUInt4(0)//0x4000)// NoClipping --maybe need these also: (eStringFormatNoFitBlackBox |StringFormatMeasureTrailingSpaces|StringFormatNoWrap|StringFormatLineLimit|StringFormatNoClip)
              << TUInt4(0) // no info about language
              << TUInt4(m_Horiz) << TUInt4(m_Vert)
              << TUInt4(1) // do not change digits based on locale
              << TUInt4(0) // no info about language
              << TFloat4(4)// tab character = this many spaces (I picked arb.)
              << TUInt4(0) //ignore hotkey prefix
              << TFloat4(0) //no leading margin
              << TFloat4(0) //no trailing margin
              << TFloat4(1) //ratio of character width to displayed space
              << TUInt4(0) //don't trim string to fit layout rectangle
              << TUInt4(0) // object doesn't provide any tab stops
              << TUInt4(0); // object doesn't provide any range data
            return o;
        }        
    };

    struct SPath : SObject {
        std::vector<SPointF> m_Points;
        std::vector<EPathPointType> m_PtType;
        std::vector<unsigned int> m_NPointsPerPoly;
        unsigned int m_TotalPts;
        
        SPath(void) : SObject(eTypePath) {
            m_TotalPts = 0;
        }
        SPath(unsigned int nPoly, double *x, double *y, int *nPts) :
        SObject(eTypePath) {
            m_NPointsPerPoly.reserve(nPoly);
            m_TotalPts = 0;
            for (unsigned int i = 0;  i < nPoly;  ++i) {
                m_NPointsPerPoly.push_back(nPts[i]);
                m_TotalPts += nPts[i];
            }
            m_Points.resize(m_TotalPts);
            for (unsigned int i = 0;  i < m_TotalPts;  ++i) {
                m_Points[i].x = x[i];
                m_Points[i].y = y[i];
            }
            m_PtType.resize(m_TotalPts, ePathPointTypeLine);
            unsigned int ptI = 0;
            for (unsigned int i = 0;  i < m_NPointsPerPoly.size();  ++i) {
                m_PtType[ptI] = ePathPointTypeStart;
                ptI += m_NPointsPerPoly[i];
            }
        }
        void StartNewPoly(double x, double y) {
            m_NPointsPerPoly.push_back(1);
            ++m_TotalPts;
            m_Points.push_back(SPointF(x, y));
            m_PtType.push_back(ePathPointTypeStart);
        }
        void AddLineTo(double x, double y) {
            if (m_NPointsPerPoly.empty()) {
                throw std::logic_error("logic error in addlineto");
            }
            ++m_NPointsPerPoly.back();
            ++m_TotalPts;
            m_Points.push_back(SPointF(x, y));
            m_PtType.push_back(ePathPointTypeLine);
        }
        void AddCubicBezierTo(double cx0, double cy0,
                              double cx1, double cy1,
                              double x, double y) {
            if (m_NPointsPerPoly.empty()) {
                throw std::logic_error("logic error in addcubicbezierto");
            }
            m_NPointsPerPoly.back() += 3;
            m_TotalPts += 3;
            m_Points.push_back(SPointF(cx0, cy0));
            m_PtType.push_back(ePathPointTypeBezier);
            m_Points.push_back(SPointF(cx1, cy1));
            m_PtType.push_back(ePathPointTypeBezier);
            m_Points.push_back(SPointF(x, y));
            m_PtType.push_back(ePathPointTypeBezier);
        }
        void AddQuadBezierTo(double cx, double cy,
                             double x, double y) {
            if (m_Points.empty()) {
                throw std::logic_error("logic error in quadbezierto");
            }
            double x0 = m_Points.back().x;
            double y0 = m_Points.back().y;
            AddCubicBezierTo(x0 + (2./3)*(cx-x0), y0 + (2./3)*(cy-y0),
                             x + (2./3)*(cx-x), y + (2./3)*(cy-y),
                             x, y);
        }
        void CloseCurrPoly(void) {
            if (!m_NPointsPerPoly.empty()  &&  m_NPointsPerPoly.back() > 0) {
                unsigned int startI = m_Points.size()-m_NPointsPerPoly.back();
                if (!(m_Points.back().x == m_Points[startI].x  &&
                      m_Points.back().y == m_Points[startI].y)) {
                    AddLineTo(m_Points[startI].x, m_Points[startI].y);
                }
            }
        }
        std::string& Serialize(std::string &o) const {
            SObject::Serialize(o);
            o << kVersion << TUInt4(m_TotalPts) << TUInt4(0);
            for (unsigned int i = 0;  i < m_TotalPts;  ++i) {
                o << m_Points[i];
            }
            unsigned int polyStart = 0;
            for (unsigned int i = 0;  i < m_NPointsPerPoly.size();  ++i) {
                for (unsigned int j = 0;  j < m_NPointsPerPoly[i];  ++j) {
                    if (j < m_NPointsPerPoly[i] - 1) { //normal point
                        o << TUInt1((0x2 << 4) | m_PtType[j+polyStart]);
                    } else {//close path
                        o << TUInt1((0x8 << 4) | m_PtType[j+polyStart]); 
                    }
                }
                polyStart += m_NPointsPerPoly[i];
            }
            return o;
        }
        friend bool operator< (const SPath& p1, const SPath& p2) {
            if (p1.m_TotalPts < p2.m_TotalPts) {
                return true;
            } else if (p1.m_TotalPts > p2.m_TotalPts) {
                return false;
            }
            int cmp = memcmp(p1.m_Points.data(),  p2.m_Points.data(),
                             sizeof(SPointF)*p1.m_TotalPts);
            if (cmp < 0) {
                return true;
            } else if (cmp > 0) {
                return false;
            }
            cmp = memcmp(p1.m_PtType.data(),  p2.m_PtType.data(),
                         sizeof(EPathPointType)*p1.m_TotalPts);
            if (cmp < 0) {
                return true;
            } else if (cmp > 0) {
                return false;
            }
            
            return (memcmp(p1.m_NPointsPerPoly.data(),
                           p2.m_NPointsPerPoly.data(),
                           sizeof(unsigned int)*p1.m_NPointsPerPoly.size())
                    < 0);
        }
    };
             
    struct SFillPolygon : SRecord {
        SColorRef m_Brush;
        unsigned int m_Count;
        SPointF *m_Points;
        SFillPolygon(int n, double *x, double *y,
                     unsigned int col) :
            SRecord(eRcdFillPolygon), m_Brush(col), m_Points(new SPointF[n]) {
            iFlags = 1 << 15; //specify solid brush, color given here
            m_Count = n;
            for (unsigned int i = 0;  i < m_Count;  ++i) {
                m_Points[i].x = x[i];
                m_Points[i].y = y[i];
            }
        }
        ~SFillPolygon(void) { delete[] m_Points; }
        std::string& Serialize(std::string &o) const {
            SRecord::Serialize(o);
            o << m_Brush << TUInt4(m_Count);
            for (unsigned int i = 0;  i < m_Count;  ++i) {
                o << m_Points[i];
            }
            return o;
	}
    };

    struct SDrawLines : SRecord {
        unsigned int count;
        SPointF *points;
        SDrawLines(int n, double *x, double *y, unsigned char penId,
                   bool close = false) :
            SRecord(eRcdDrawLines), points(new SPointF[n+(close?1:0)]) {
            iFlags = penId;
            count = n + (close?1:0);
            for (int i = 0;  i < n;  ++i) {
                points[i].x = x[i];
                points[i].y = y[i];
            }
            if (close) {
                points[n].x = x[0];
                points[n].y = y[0];
            }
        }
        ~SDrawLines(void) { delete[] points; }
        std::string& Serialize(std::string &o) const {
            SRecord::Serialize(o) << TUInt4(count);
            for (unsigned int i = 0;  i < count;  ++i) {
                o << points[i];
            }
            return o;
	}
    };

    struct SFillEllipse : SRecord {
        TUInt4 m_BrushId;
        SColorRef m_Col;
        bool m_SimpleBrush;
        SRectF rect;
        SFillEllipse(double x, double y, double w, double h,
                  unsigned char r, unsigned char g, unsigned char b,
                  unsigned char a) : SRecord(eRcdFillEllipse) {
            iFlags = 1 << 15; //specify solid brush, color given here
            rect.x = x; rect.y = y; rect.w = w; rect.h = h;
            m_Col.Set(r,g,b,a);
            m_SimpleBrush = true;
        }
        SFillEllipse(double x, double y, double w, double h,
                     unsigned char brushId) : SRecord(eRcdFillEllipse) {
            iFlags = 0;
            rect.x = x; rect.y = y; rect.w = w; rect.h = h;
            m_BrushId = brushId;
            m_SimpleBrush = false;
        }
        std::string& Serialize(std::string &o) const {
            if (m_SimpleBrush) {
                return SRecord::Serialize(o) << m_Col << rect;
            } else {
                return SRecord::Serialize(o) << m_BrushId << rect;
            }
	}
    };

    struct SDrawEllipse : SRecord {
        SRectF rect;
        SDrawEllipse(double x, double y, double w, double h,
                     unsigned char penId) : SRecord(eRcdDrawEllipse) {
            iFlags = penId;
            rect.x = x; rect.y = y; rect.w = w; rect.h = h;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << rect;
	}
    };

    struct SDrawPath : SRecord {
        TUInt4 m_PenId;
        SDrawPath(unsigned char pathId,
                  unsigned char penId) : SRecord(eRcdDrawPath) {
            iFlags = pathId;
            m_PenId = penId;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << m_PenId;
	}
    };

    struct SFillPath : SRecord {
        TUInt4 m_BrushId;
        SColorRef m_Col;
        bool m_SimpleBrush;
        SFillPath(unsigned char pathId,
                  unsigned char r, unsigned char g, unsigned char b,
                  unsigned char a) : SRecord(eRcdFillPath) {
            iFlags = 1 << 15 | pathId;
            m_Col.Set(r,g,b,a);
            m_SimpleBrush = true;
        }
        SFillPath(unsigned char pathId,
                  unsigned char brushId) : SRecord(eRcdFillPath) {
            iFlags = pathId;
            m_BrushId = brushId;
            m_SimpleBrush = false;
        }
        std::string& Serialize(std::string &o) const {
            if (m_SimpleBrush) {
                return SRecord::Serialize(o) << m_Col;
            } else {
                return SRecord::Serialize(o) << m_BrushId;
            }
	}
    };

    struct SDrawString : SRecord {
        SColorRef m_Brush;
        unsigned char m_StringFormatId;
        SRectF m_LayoutRect;
        std::string m_StringUTF16LE;
        SDrawString(const std::string &s,
                    unsigned int col, unsigned char fontId,
                    unsigned char stringFormatId) :
            SRecord(eRcdDrawString), m_Brush(col) {
            iFlags = 1 << 15 | fontId; //bit indicates color specified here
            m_StringFormatId = stringFormatId;
            m_StringUTF16LE = s;
        }
        std::string& Serialize(std::string &o) const {
            SRecord::Serialize(o) << m_Brush << TUInt4(m_StringFormatId)
                                  << TUInt4(m_StringUTF16LE.length()/2)
                                  << m_LayoutRect;
            o.append(m_StringUTF16LE);
            return o;
	}
    };

    struct SDrawImage : SRecord {
        SRectF m_SrcRect;
        SRectF m_Rect;
        SDrawImage(unsigned char imageId, int srcW, int srcH,
                   double x, double y, double w, double h) :
        SRecord(eRcdDrawImage) {
            iFlags = imageId;
            m_SrcRect.x = m_SrcRect.y=0;
            m_SrcRect.w = srcW;
            m_SrcRect.h = srcH;
            m_Rect.x = x;
            m_Rect.y = y;
            m_Rect.w = w;
            m_Rect.h = h;
        }
        std::string& Serialize(std::string &o) const {
            return SRecord::Serialize(o) << TUInt4(0) << TUInt4(eUnitPixel)
                                         << m_SrcRect << m_Rect;
	}
    };

    struct SImage : SObject {
        unsigned int m_W, m_H;
        std::string m_RawARGB;
        SImage(unsigned int *data, unsigned int w, unsigned int h) :
        SObject(eTypeImage) {
            m_W = w;
            m_H = h;
            m_RawARGB.resize(w*h*4);
            for (unsigned int i = 0;  i < m_W*m_H; ++i) {
                m_RawARGB[4*i+0] = R_BLUE(data[i]);
                m_RawARGB[4*i+1] = R_GREEN(data[i]);
                m_RawARGB[4*i+2] = R_RED(data[i]);
                m_RawARGB[4*i+3] = R_ALPHA(data[i]);
            }
        }
        std::string& Serialize(std::string &o) const {
            SObject::Serialize(o) << kVersion << TUInt4(1) <<
                TUInt4(m_W) << TUInt4(m_H) << TUInt4(4*m_W) <<
                TUInt4(0x26200A) <<
                //TUInt4(32 << 16 | 10 << 24) << 
                TUInt4(0);
            o.append(m_RawARGB);
            return o;
	}
    };

    SPen::SPen(unsigned int col, double lwd, unsigned int lty,
               unsigned int lend, unsigned int ljoin, unsigned int lmitre,
               double ps2dev, bool useUserLty) : SObject(eTypePen), brush(col) {
        pen.width = lwd*ps2dev;
        if (!useUserLty) {
            // if not using EMF custom line types, then map
            // between vaguely equivalent default line types
            switch(lty) {
            case LTY_SOLID: pen.lineStyle = eLineSolid; break;
            case LTY_DASHED: pen.lineStyle = eLineDash; break;
            case LTY_DOTTED: pen.lineStyle = eLineDot; break;
            case LTY_DOTDASH: pen.lineStyle = eLineDashDot; break;
            case LTY_LONGDASH: pen.lineStyle = eLineDashDotDot; break;
            default: pen.lineStyle = eLineSolid;
                Rf_warning("Requested lty is unsupported by EMF device without "
                           "custom line types (see option to 'emf' function)");
            }
        } else { //custom line style is preferable
            for(int i = 0;  i < 8  &&  lty & 15;  ++i, lty >>= 4) {
                pen.dashedLineData.push_back(lty & 15);
            }
            if (pen.dashedLineData.empty()) {
                pen.lineStyle = eLineSolid;
            } else {
                pen.lineStyle = eLineCustom;
            }
        }

        switch (lend) {
        case GE_ROUND_CAP: pen.startCap = eCapRound; break;
        case GE_BUTT_CAP: pen.startCap = eCapFlat; break;
        case GE_SQUARE_CAP: pen.startCap = eCapSquare; break;
        default: break;//actually of range, but R doesn't complain..
        }
        pen.dashedCap = pen.endCap = pen.startCap;

        switch (ljoin) {
        case GE_ROUND_JOIN: pen.join = eJoinRound; break;
        case GE_MITRE_JOIN: pen.join = eJoinMiter; break;
        case GE_BEVEL_JOIN: pen.join = eJoinBevel; break;
        default: break;//actually of range, but R doesn't complain..
        }

        pen.miterLimit = lmitre;
    }

    struct ObjectPtrCmp {
        bool operator() (const SObject* o1, const SObject* o2) const {
            if (o1->type < o2->type) {
                return true;
            } else if (o1->type > o2->type) {
                return false;
            } else { //same type -- so compare details
                switch (o1->type) {
                case eTypeBrush:
                    return *dynamic_cast<const SBrush*>(o1) <
                        *dynamic_cast<const SBrush*>(o2);
                case eTypePen: {
                    const SPen *p1 = dynamic_cast<const SPen*>(o1);
                    const SPen *p2 = dynamic_cast<const SPen*>(o2);
                    int cmp = memcmp(&p1->pen, &p2->pen, 28);//not ideal..
                    if (cmp == 0) {
                        if (p1->pen.dashedLineData < p2->pen.dashedLineData)
                            return true;
                        else if (p1->pen.dashedLineData >
                                 p2->pen.dashedLineData)
                            return false;
                    }

                    return (cmp < 0  ||
                            (cmp == 0  &&
                             memcmp(&p1->brush, &p2->brush,
                                    sizeof(p1->brush)) < 0));
                }
                case eTypeFont: {
                    const SFont* f1 = dynamic_cast<const SFont*>(o1);
                    const SFont* f2 = dynamic_cast<const SFont*>(o2);
                    return f1->m_emSize < f2->m_emSize  ||
                        (f1->m_emSize == f2->m_emSize  &&
                         (f1->m_Style < f2->m_Style  ||
                          (f1->m_Style == f2->m_Style  &&
                           f1->m_FamilyUTF16 < f2->m_FamilyUTF16)));
                }
                case eTypeStringFormat: {
                    const SStringFormat* s1 =
                        dynamic_cast<const SStringFormat*>(o1);
                    const SStringFormat* s2 =
                        dynamic_cast<const SStringFormat*>(o2);
                    return s1->m_Horiz < s2->m_Horiz  ||
                        (s1->m_Horiz == s2->m_Horiz  &&
                         s1->m_Vert < s2->m_Vert);
                }
                case eTypePath: {
                    return *dynamic_cast<const SPath*>(o1) <
                        *dynamic_cast<const SPath*>(o2);
                }
                case eTypeImage: {
                    return false; //assume images are unique
                }
                default: {//should never happen!
                    throw std::logic_error("EMF+ object table scrambled");
                }
                }
            }
        }
    };


    class CObjectTable {
    public:
        CObjectTable(void) {
            m_LastInserted = kMaxObjTableSize-1;
            memset(m_Table, 0, sizeof(m_Table));
        }
        ~CObjectTable(void) {
            for (unsigned int i = 0;  i < kMaxObjTableSize;  ++i) {
                delete m_Table[i];
            }
        }

        unsigned char GetPen(unsigned int col, double lwd, unsigned int lty,
                             unsigned int lend, unsigned int ljoin,
                             unsigned int lmitre, double ps2dev,
                             bool useUserLty, EMF::ofstream &out) {
            SPen *pen = new SPen(col, lwd, lty, lend, ljoin, lmitre, ps2dev,
                                 useUserLty);
            return x_InsertObject(pen, out);
        }
        unsigned char GetBrush(SBrush* brush, EMF::ofstream &out) {
            return x_InsertObject(brush, out);
        }
        unsigned char GetBrush(unsigned int col, EMF::ofstream &out) {
            SBrush *brush = new SBrush(col);
            return x_InsertObject(brush, out);
        }
        unsigned char GetFont(unsigned char face, double size,
                              const std::string &familyUTF16,
                              EMF::ofstream &out) {
            SFont *font = new SFont(face, size, familyUTF16);
            return x_InsertObject(font, out);
        }
        unsigned char GetStringFormat(EStringAlign h, EStringAlign v,
                                      EMF::ofstream &out) {
            SStringFormat *fmt = new SStringFormat(h, v);
            return x_InsertObject(fmt, out);
        }
        unsigned char GetPath(SPath* path, EMF::ofstream &out) {
            return x_InsertObject(path, out);
        }
        unsigned char GetImage(unsigned int *data, int w, int h,
                              EMF::ofstream &out) {
            SImage *image = new SImage(data, w, h);
            return x_InsertObject(image, out);
        }
    private:
        //note: takes ownership over pointer!
        unsigned char x_InsertObject(SObject *obj, EMF::ofstream &out) {
            TIndex::iterator i = m_Index.find(obj);
            if (i == m_Index.end()) {
                m_LastInserted = (m_LastInserted+1) % kMaxObjTableSize;
                SObject *old = m_Table[m_LastInserted];
                if (old) {
                    m_Index.erase(old);
                    delete old;
                }
                m_Table[m_LastInserted] = obj;
                obj->SetObjId(m_LastInserted);
                i = m_Index.insert(obj).first;
                obj->Write(out);
            } else {
                delete obj;
            }
            return (*i)->GetObjId();
        }
    private:
        SObject* m_Table[kMaxObjTableSize];
        unsigned int m_LastInserted;
        typedef std::set<SObject*, ObjectPtrCmp> TIndex;
        TIndex m_Index;
    };
} //end of EMFPLUS namespace
