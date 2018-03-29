#include "swf_decode.h"
#include "swf_types.h"
#include "swf_tags.h"
#include "tag_decoder.h"
#include <iostream>
#include <assert.h>

namespace swf_redux
{

namespace swf
{

inline uint32_t Fixed16_16(int16_t i, int16_t f)
{
    return (i << 16) | f;
}

void DecodeStyleChange(TagDecoder & io, StyleChangedRecord & stylechange, ShapeWithStyle & a_Shape);
void DecodeStyleChange(TagDecoder & io, StyleChangedRecord & stylechange, Shape & a_Shape);

/*************************************************************************/
/*									Basic types							 */
/*************************************************************************/
template<>
void Decode(io::Source & io, RGBA & rgba)
{
    rgba.r = io.get_u8();
    rgba.g = io.get_u8();
    rgba.b = io.get_u8();
    rgba.a = io.get_u8();
}

template<>
void Decode(io::Source & io, RGB & rgb)
{
    rgb.r = io.get_u8();
    rgb.g = io.get_u8();
    rgb.b = io.get_u8();
}

template<>
void Decode(io::Source & io, FileHeader & header)
{
    
    Decode<RECT>(io, header.FrameSize);

    header.FrameRate	= io.get_u16();
    header.FrameCount	= io.get_u16();
}

template<>
void Decode(io::Source & io, RECT & rect)
{
    io.assure_alignment();
    uint8_t NBits = static_cast<uint8_t>(io.get_bits(5));
    /** Sign-extend and read the coordinates */
    rect.Xmin = io.get_signed(NBits);
    rect.Xmax = io.get_signed(NBits);
    rect.Ymin = io.get_signed(NBits);
    rect.Ymax = io.get_signed(NBits);
    /** align the io to byte boundry */
    io.align();
}

template<>
void Decode(io::Source & io, Matrix_Fixed2x3 & matrix)
{
    io.assure_alignment();
    /** Scaling */
    matrix.HasScale = io.get_bits(1);
    if (matrix.HasScale) {
        uint8_t NScaleBits = io.get_bits(5);
        matrix.ScaleX = io.get_fixed16(NScaleBits);
        matrix.ScaleY = io.get_fixed16(NScaleBits);
    } else {
        matrix.ScaleX = Fixed16_16(1, 0);
        matrix.ScaleY = Fixed16_16(1, 0);
    }
    /** rotate */
    matrix.HasRotate = io.get_bits(1);
    if (matrix.HasRotate) {
        uint8_t NRotateBits = io.get_bits(5);
        matrix.RotateSkew0 = io.get_fixed16(NRotateBits);
        matrix.RotateSkew1 = io.get_fixed16(NRotateBits);
    } else {
        matrix.RotateSkew0 = matrix.RotateSkew1 = Fixed16_16(0, 0);
    }
    /** translate */
    uint8_t NTranslateBits = io.get_bits(5);
    matrix.TranslateX = io.get_signed(NTranslateBits);
    matrix.TranslateY = io.get_signed(NTranslateBits);
    io.align();
}

/*************************************************************************/
/*									Shapes								 */
/*************************************************************************/
template<>
void Decode(TagDecoder & io, StraightEdgeRecord & straight)
{
    size_t NumBits = io.get_bits(4);
    uint8_t	GeneralLineFlag = io.get_bits(1);
    uint8_t VertLineFlag = (GeneralLineFlag ? 0 : io.get_bits(1));
    straight.DeltaX = (GeneralLineFlag || (!VertLineFlag)) ? io.get_signed(NumBits+2) : 0;
    straight.DeltaY = (GeneralLineFlag || (VertLineFlag)) ? io.get_signed(NumBits+2) : 0;
}

/**
* Decodes a Curved edge record which defines a Quadratic Bezier curve.
*/
template<>
void Decode<CurvedEdgeRecord>(TagDecoder & io, CurvedEdgeRecord & curved)
{
    uint8_t	NumBits			= io.get_bits(4) + 2;
    curved.ControlDeltaX	= io.get_signed(NumBits);
    curved.ControlDeltaY	= io.get_signed(NumBits);
    curved.AnchorDeltaX		= io.get_signed(NumBits);
    curved.AnchorDeltaY		= io.get_signed(NumBits);
}

/**
* Decodes a shape record.
*/
void DecodeShapeRecord(TagDecoder & io, ShapeRecord & a_Record, ShapeWithStyle & a_Shape)
{
    if (io.get_bits(1)) {		/** edge record */
        if (io.get_bits(1)) {
            a_Record.m_RecordType = ShapeRecord::STRAIGHT_EDGE_RECORD;
            Decode<StraightEdgeRecord>(io, a_Record.u.StraightEdge);
        } else {
            a_Record.m_RecordType = ShapeRecord::CURVED_EDGE_RECORD;
            Decode<CurvedEdgeRecord>(io, a_Record.u.CurvedEdge);
        }
    } else {

        a_Record.u.StyleChanged.cflags			= 0;
        a_Record.u.StyleChanged.StateNewStyles	= io.get_bits(1);
        a_Record.u.StyleChanged.StateLineStyle		= io.get_bits(1);
        a_Record.u.StyleChanged.StateFillStyle1		= io.get_bits(1);
        a_Record.u.StyleChanged.StateFillStyle0		= io.get_bits(1);
        a_Record.u.StyleChanged.StateMoveTo		= io.get_bits(1);

        if (a_Record.u.StyleChanged.cflags == 0) {
            /** this is a end record */
            a_Record.m_RecordType = ShapeRecord::END_SHAPE_RECORD;
        } else {
            /** we need to supply this information when parsing the style change record */
            a_Record.u.StyleChanged.NumFillBits		= a_Record.NumFillBits;
            a_Record.u.StyleChanged.NumLineBits		= a_Record.NumLineBits;
            a_Record.u.StyleChanged.LineStyleOffset	= a_Record.LineStyleOffset;
            a_Record.u.StyleChanged.FillStyleOffset	= a_Record.FillStyleOffset;

            DecodeStyleChange(io, a_Record.u.StyleChanged, a_Shape);
            if (a_Record.u.StyleChanged.StateNewStyles) {
                a_Record.NumFillBits = a_Record.u.StyleChanged.NumFillBits;
                a_Record.NumLineBits = a_Record.u.StyleChanged.NumLineBits;
            }
            a_Record.m_RecordType = ShapeRecord::STYLE_CHANGE_RECORD;
            a_Record.FillStyleOffset = a_Record.u.StyleChanged.FillStyleOffset;
            a_Record.LineStyleOffset = a_Record.u.StyleChanged.LineStyleOffset;
        }
    }
}

/**
* Decodes a shape record.
*/
void DecodeShapeRecord(TagDecoder & io, ShapeRecord & a_Record, Shape & a_Shape)
{
    if (io.get_bits(1)) {		/** edge record */
        if (io.get_bits(1)) {
            a_Record.m_RecordType = ShapeRecord::STRAIGHT_EDGE_RECORD;
            Decode<StraightEdgeRecord>(io, a_Record.u.StraightEdge);
        } else {
            a_Record.m_RecordType = ShapeRecord::CURVED_EDGE_RECORD;
            Decode<CurvedEdgeRecord>(io, a_Record.u.CurvedEdge);
        }
    } else {
        a_Record.u.StyleChanged.cflags			= 0;
        a_Record.u.StyleChanged.StateNewStyles	= io.get_bits(1);
        a_Record.u.StyleChanged.StateLineStyle	= io.get_bits(1);
        a_Record.u.StyleChanged.StateFillStyle1	= io.get_bits(1);
        a_Record.u.StyleChanged.StateFillStyle0	= io.get_bits(1);
        a_Record.u.StyleChanged.StateMoveTo		= io.get_bits(1);

        if (a_Record.u.StyleChanged.cflags == 0) {
            /** this is a end record */
            a_Record.m_RecordType = ShapeRecord::END_SHAPE_RECORD;
        } else {
            /** we need to supply this information when parsing the style change record */
            a_Record.u.StyleChanged.NumFillBits		= a_Record.NumFillBits;
            a_Record.u.StyleChanged.NumLineBits		= a_Record.NumLineBits;
            a_Record.u.StyleChanged.LineStyleOffset	= a_Record.LineStyleOffset;
            a_Record.u.StyleChanged.FillStyleOffset	= a_Record.FillStyleOffset;

            DecodeStyleChange(io, a_Record.u.StyleChanged, a_Shape);
            a_Record.m_RecordType = ShapeRecord::STYLE_CHANGE_RECORD;
        }
    }
}

template<>
void Decode<GRADRECORD>(TagDecoder & io, GRADRECORD & gr)
{
    gr.ratio = io.get_u8();
    uint16_t tagCode = io.tag_code();
    if (tagCode == SwfTag_DefineShape || tagCode == SwfTag_DefineShape2) {
        Decode((io::Source &) io, gr.color.rgb);
        gr.color.rgba.a = 255;
    } else {
        Decode((io::Source &) io, gr.color.rgba);
    }
}

template<>
void Decode<Filter>(io::Source & io, Filter & a_Filter)
{
    a_Filter.Type = io.get_u8();
    switch(a_Filter.Type) 
    {
    case 0:	Decode(io, a_Filter.dropShadow); break;
    case 1: Decode(io, a_Filter.blur); break;
    case 2: Decode(io, a_Filter.glow); break;
    case 6: Decode(io, a_Filter.colorMatrix); break;
    default: throw std::runtime_error("Filter type not implemented.");
    }
}

template<>
void Decode<DropShadowFilter>(io::Source & io, DropShadowFilter & drop)
{
    Decode(io, drop.DropShadowColor);
    drop.BlurX				= io.get_fixed16(32);
    drop.BlurY				= io.get_fixed16(32);
    drop.Angle				= io.get_fixed16(32);
    drop.Distance			= io.get_fixed16(32);
    drop.Strength			= io.get_fixed16(16); // TODO: FIX, should be FIXED8
    drop.InnerShadow		= io.get_bits(1);
    drop.Knockout			= io.get_bits(1);
    drop.CompositeSource	= io.get_bits(1);
    drop.Passes				= io.get_bits(5);
}

template<>
void Decode<BlurFilter>(io::Source & io, BlurFilter & blur)
{
    blur.BlurX				= io.get_fixed16(32);
    blur.BlurY				= io.get_fixed16(32);
    blur.Passes				= io.get_bits(5);
    io.get_bits(3);
}

template<>
void Decode<GlowFilter>(io::Source & io, GlowFilter & glow)
{
    Decode(io, glow.glowColor);
    glow.BlurX              = io.get_fixed16(32);
    glow.BlurY              = io.get_fixed16(32);
    glow.Strength           = io.get_fixed8();
    glow.InnerGlow          = io.get_bits(1);
    glow.KnockOut           = io.get_bits(1);
    glow.CompositeSource    = io.get_bits(1);
    glow.Passes             = io.get_bits(3);
}

template<>
void Decode<ColorMatrixFilter>(io::Source & io, ColorMatrixFilter & colorMatrix)
{
    for(size_t i = 0; i < 20; ++i)
    {
        colorMatrix.Matrix[i] = io.get_float();
    }
}

template<>
void Decode<FillStyle>(TagDecoder & io, FillStyle & fs)
{
    uint16_t tagCode = io.tag_code();
    fs.FillStyleType = io.get_u8();
    switch(fs.FillStyleType) {
    case 0x00: /** solid fill */
        if ((io.tag_code() == SwfTag_DefineShape) || (io.tag_code() == SwfTag_DefineShape2)) { 
            Decode<RGB>((io::Source &)io, fs.color.rgb);
            fs.color.rgba.a = 255;
        } else {
            Decode<RGBA>((io::Source &)io, fs.color.rgba);
        }
        fs.FillType = FillStyle::eSOLID_RGBA;
        break;
    case 0x10: /** linear gradient fill */
    case 0x12:
        {
            Matrix_Fixed2x3 mat;
            Decode((io::Source &)io, mat);
            uint8_t SpreadMode = io.get_bits(2);
            uint8_t interpolationMode = io.get_bits(2);
            if ((SpreadMode != 0) || (interpolationMode != 0)) {
                if ((tagCode == SwfTag_DefineShape) || 
                    (tagCode == SwfTag_DefineShape2) || 
                    (tagCode == SwfTag_DefineShape3)) 
                {
                    throw std::runtime_error("The SpreadMode/InterpolationMode parameter of the GRADIENT record must be 0 for this tag");
                }
            }
            uint8_t NumGradients = io.get_bits(4);
            if (NumGradients > 8) {
                if ((tagCode == SwfTag_DefineShape) || 
                    (tagCode == SwfTag_DefineShape2) || 
                    (tagCode == SwfTag_DefineShape3)) 
                {
                    throw std::runtime_error("The NumGradients field cannot exceed 8 for this tag");
                }
            }
            if (fs.FillStyleType == 0x10) {
                fs.FillType	= FillStyle::eLINEAR_GRADIENT;
            } else {
                fs.FillType	= FillStyle::eRADIAL_GRADIENT;
            }
            fs.LinearGradient.numControlPoints  = NumGradients;
            for(size_t i = 0; i < NumGradients; ++i) {
                /** decode the gradient control points */
                Decode(io, fs.LinearGradient.controlPoint[i]);
            }
            break;
        }
    case 0x13: /**< focal radial fill */
        {
            Matrix_Fixed2x3 mat;
            Decode((io::Source &)io, mat);
            uint8_t SpreadMode = io.get_bits(2);
            uint8_t interpolationMode = io.get_bits(2);
            if ((SpreadMode != 0) || (interpolationMode != 0)) {
                if ((tagCode == SwfTag_DefineShape) || 
                    (tagCode == SwfTag_DefineShape2) || 
                    (tagCode == SwfTag_DefineShape3)) 
                {
                    throw std::runtime_error("The SpreadMode/InterpolationMode parameter of the GRADIENT record must be 0 for this tag");
                }
            }
            uint8_t NumGradients = io.get_bits(4);
            if (NumGradients > 8) {
                if ((tagCode == SwfTag_DefineShape) || 
                    (tagCode == SwfTag_DefineShape2) || 
                    (tagCode == SwfTag_DefineShape3)) 
                {
                    throw std::runtime_error("The NumGradients field cannot exceed 8 for this tag");
                }
            }
            fs.FillType							= FillStyle::eFOCAL_GRADIENT;
            fs.FocalGradient.numControlPoints  = NumGradients;
            for(size_t i = 0; i < NumGradients; ++i) {
                /** decode the gradient control points */
                Decode(io, fs.FocalGradient.controlPoint[i]);
            }
            fs.FocalGradient.focalPoint = io.get_fixed8();
            break;
        }
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
        if (fs.FillStyleType == 0x40)		fs.FillType	= FillStyle::eREPEAT_BITMAP;
        else if(fs.FillStyleType == 0x41)	fs.FillType	= FillStyle::eCLIPPED_BITMAP;
        else if(fs.FillStyleType == 0x42)	fs.FillType	= FillStyle::eNON_SMOOTH_REPEAT_BITMAP;
        else if(fs.FillStyleType == 0x43)	fs.FillType	= FillStyle::eNON_SMOOTH_CLIPPED_BITMAP;

        fs.Bitmap.bitmapId = io.get_u16();
        Decode((io::Source&) io, fs.Bitmap.matrix);
        break;
    default:
        assert(false);
    }
}

/**
* Solid fill color with opacity information.
*/
template<>
void Decode<LineStyle>(TagDecoder & io, LineStyle & ls)
{
    uint16_t tagCode = io.tag_code();
    if ((tagCode == SwfTag_DefineShape) || (tagCode == SwfTag_DefineShape2) || (tagCode == SwfTag_DefineShape3)) {
        ls.type = LineStyle::LINESTYLE1;
        ls.LineStyle1.width = io.get_u16();
        if ((tagCode == SwfTag_DefineShape) || (tagCode == SwfTag_DefineShape2)) {
            Decode<RGB>((io::Source &)io, ls.LineStyle1.color.rgb);
            ls.LineStyle1.color.rgba.a = 255;
        } else {
            Decode<RGBA>((io::Source &)io, ls.LineStyle1.color.rgba);
        }
    } else {
        ls.type = LineStyle::LINESTYLE2;
        ls.Linestyle2.Width			= io.get_u16();
        ls.Linestyle2.StartCapStyle = io.get_bits(2);
        ls.Linestyle2.JoinStyle		= io.get_bits(2);
        ls.Linestyle2.HasFillFlag	= io.get_bits(1);
        ls.Linestyle2.NoHScaleFlag	= io.get_bits(1);
        ls.Linestyle2.NoVScaleFlag	= io.get_bits(1);
        ls.Linestyle2.PixelHintingFlag = io.get_bits(1);
        io.get_bits(5); /**< 5 reserved bits */
        ls.Linestyle2.NoClose		= io.get_bits(1);
        ls.Linestyle2.EndCapStyle	= io.get_bits(2);
        ls.Linestyle2.MiterLimitFactor = (ls.Linestyle2.JoinStyle == 2) ? io.get_u16() : 0;
        if(!ls.Linestyle2.HasFillFlag) {
            Decode((io::Source &)io, ls.Linestyle2.Color.rgba);
        } else {
            Decode(io, ls.Linestyle2.FillType);
        }
    }
}

/**
* Decodes a shape definition used by DefineShape, DefineShape2 and DefineShape3.
*/
template<>
void Decode<ShapeWithStyle>(TagDecoder & io, ShapeWithStyle & shape)
{
    DecodeArray<FillStyle>(io, shape.FillStyles);
    DecodeArray<LineStyle>(io, shape.LineStyles);
    io.assure_alignment();

    /** read shape records until a end shape record is reached */
    ShapeRecord record;
    record.NumFillBits = io.get_bits(4);
    record.NumLineBits = io.get_bits(4);
    record.FillStyleOffset = 0;
    record.LineStyleOffset = 0;

    record.u.StyleChanged.LineStyle		= -1;
    record.u.StyleChanged.FillStyle0	= -1;
    record.u.StyleChanged.FillStyle1	= -1;

    do {
        DecodeShapeRecord(io, record, shape);
        shape.Shapes.push_back(record);
    } while(record.m_RecordType != ShapeRecord::END_SHAPE_RECORD);
    io.align();
}

template<>
void Decode<Shape>(TagDecoder & io, Shape & shape)
{
    /** read shape records until a end shape record is reached */
    ShapeRecord record;
    record.NumFillBits = io.get_bits(4);
    record.NumLineBits = io.get_bits(4);
    record.FillStyleOffset = 0;
    record.LineStyleOffset = 0;

    do {
        DecodeShapeRecord(io, record, shape);
        shape.Shapes.push_back(record);
    } while(record.m_RecordType != ShapeRecord::END_SHAPE_RECORD);
}

void DecodeStyleChange(TagDecoder & io, 
    StyleChangedRecord & stylechange,
    ShapeWithStyle & a_Shape)
{
    if (stylechange.StateMoveTo) {
        uint8_t MoveBits		= io.get_bits(5);
        stylechange.MoveDeltaX	= io.get_signed(MoveBits);
        stylechange.MoveDeltaY	= io.get_signed(MoveBits);
    } else {
        stylechange.MoveDeltaX	= 0;
        stylechange.MoveDeltaY	= 0;
    }
    if ((stylechange.StateFillStyle0) && (stylechange.NumFillBits > 0)) {
        stylechange.FillStyle0 = io.get_bits(stylechange.NumFillBits) + stylechange.FillStyleOffset;
    }
    if ((stylechange.StateFillStyle1) && (stylechange.NumFillBits > 0)) {
        stylechange.FillStyle1 = io.get_bits(stylechange.NumFillBits) + stylechange.FillStyleOffset;
    }
    if ((stylechange.StateLineStyle) && (stylechange.NumLineBits > 0)) {
        stylechange.LineStyle = io.get_bits(stylechange.NumLineBits) + stylechange.LineStyleOffset;
    }
    if (stylechange.StateNewStyles) {
        if((io.tag_code() != SwfTag_DefineShape2) &&
            (io.tag_code() != SwfTag_DefineShape3) &&
            (io.tag_code() != SwfTag_DefineShape4)) 
        {
            return;
        }
        /** read the new fill and linestyles */
        size_t numFillStyles = a_Shape.FillStyles.size();
        size_t numLineStyles = a_Shape.LineStyles.size();
        DecodeArray<FillStyle>(io, a_Shape.FillStyles);
        DecodeArray<LineStyle>(io, a_Shape.LineStyles);

        stylechange.NumFillBits = io.get_bits(4);
        stylechange.NumLineBits = io.get_bits(4);

        stylechange.LineStyleOffset = numLineStyles;
        stylechange.FillStyleOffset	= numFillStyles;

        stylechange.LineStyle	= -1;
        stylechange.FillStyle0	= -1;
        stylechange.FillStyle1	= -1;
    }
}

void DecodeStyleChange(TagDecoder & io, StyleChangedRecord & stylechange, Shape & a_Shape)
{
    if (stylechange.StateMoveTo) {
        uint8_t MoveBits		= io.get_bits(5);
        stylechange.MoveDeltaX	= io.get_signed(MoveBits);
        stylechange.MoveDeltaY	= io.get_signed(MoveBits);
    } else {
        stylechange.MoveDeltaX	= 0;
        stylechange.MoveDeltaY	= 0;
    }
    if (stylechange.StateFillStyle0) {
        stylechange.FillStyle0 = io.get_bits(stylechange.NumFillBits) + stylechange.FillStyleOffset;
    }
    if (stylechange.StateFillStyle1) {
        stylechange.FillStyle1 = io.get_bits(stylechange.NumFillBits) + stylechange.FillStyleOffset;
    }
    if (stylechange.StateLineStyle) {
        stylechange.LineStyle = io.get_bits(stylechange.NumLineBits) + stylechange.LineStyleOffset;
    }
}

template<>
void Decode<CXFormWithAlpha>(io::Source & io, CXFormWithAlpha & cxform)
{
    io.align();
    cxform.HasAddTerms	= io.get_bits(1);
    cxform.HasMultTerms	= io.get_bits(1);

    uint8_t Nbits = io.get_bits(4);
    if (cxform.HasMultTerms) {
        cxform.RedMultTerm = io.get_signed(Nbits);
        cxform.GreenMultTerm = io.get_signed(Nbits);
        cxform.BlueMultTerm = io.get_signed(Nbits);
        cxform.AlphaMultTerm = io.get_signed(Nbits);
    }
    if (cxform.HasAddTerms) {
        cxform.RedAddTerm = io.get_signed(Nbits);
        cxform.GreenAddTerm = io.get_signed(Nbits);
        cxform.BlueAddTerm = io.get_signed(Nbits);
        cxform.AlphaAddTerm = io.get_signed(Nbits);
    }
}

template<>
void Decode<MorphFillStyle>(TagDecoder & io, 
    MorphFillStyle & a_Style)
{
    switch(io.get_u8())
    {
    case 0x00:
        Decode((io::Source&)io, a_Style.solid.StartColor);
        Decode((io::Source&)io, a_Style.solid.EndColor);
        break;
    default:
        throw std::runtime_error("TODO: This morph style has not been implemented.");
    }
}

template<>
void Decode<MorphLineStyle>(TagDecoder & io, MorphLineStyle & a_Style)
{
    if (io.tag_code() == SwfTag_DefineMorphShape2) {
        a_Style.IsVersion2              = 1;
        a_Style.version2.StartWidth		= io.get_u16();
        a_Style.version2.EndWidth		= io.get_u16();
        a_Style.version2.StartCapStyle	= io.get_bits(2);
        a_Style.version2.JoinStyle		= io.get_bits(2);
        a_Style.version2.HasFillFlag	= io.get_bits(1);
        a_Style.version2.NoHScaleFlag	= io.get_bits(1);
        a_Style.version2.NoVScaleFlag	= io.get_bits(1);
        a_Style.version2.PixelHintingFlag	= io.get_bits(1);
        io.get_bits(5); /**< reserved */

        a_Style.version2.NoClose		= io.get_bits(1);
        a_Style.version2.EndCapStyle	= io.get_bits(2);
        if (a_Style.version2.JoinStyle == 2) {
            a_Style.version2.MiterLimitFactor = io.get_u16();
        }
        if (a_Style.version2.HasFillFlag == 0) {
            Decode((io::Source &)io, a_Style.version2.StartColor);
            Decode((io::Source &)io, a_Style.version2.EndColor);
        } else {
            throw std::runtime_error("Support for filled morph lines not implemented.");
        }
    } else {
        a_Style.IsVersion2 = 0;
        a_Style.version1.StartWidth = io.get_u16();
        a_Style.version1.EndWidth = io.get_u16();
        Decode((io::Source &)io, a_Style.version1.StartColor);
        Decode((io::Source &)io, a_Style.version1.EndColor);
    }
}

}

} // namespace swf_redux