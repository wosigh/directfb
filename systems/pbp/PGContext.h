/* ============================================================
 * Date  : 2008-08-07
 * Copyright 2008 Palm, Inc. All rights reserved.
 * ============================================================ */

#ifndef PGCONTEXT_H
#define PGCONTEXT_H

#include <stdint.h>
#include <PContext.h>

#include "PGShared.h"
#include "PGContextIface.h"

class PGSurface;
class PGFont;
struct PGContextState;

class PGEXPORT PGContext : public PGContextIface
{
public:

	static PGContext* create();
	static void downSample(PGSurface* dst, PGSurface* src);
	
	PContext2D* pgContext() const { return m_gc; }
	
	PGSurface* createSurface(uint32_t width, uint32_t height,
							 bool hasAlpha, const void* pixelData);
	PGSurface* wrapBitmapData(uint32_t width, uint32_t height,
							  bool hasAlpha, const void* pixelData);
	void destroySurface(PGSurface* surf);


	void setSurface(PGSurface* surf);
	PGSurface* getSurface() const;

	void setFont(PGFont* font);
	PGFont* font() const;
	void setFallbackFont(PGFont* font);
	void setFallbackFonts(PGFont** fonts, int numFonts);

	// clears out the gfx stack
	void reset();

	void push();
	void pop();

	int originX() const;
	int originY() const;

	void translate(int x, int y);
	void rotate(PValue radians);
	void scale(PValue x, PValue y);

	void setFillColor(const PColor32& color);
	void setStrokeColor(const PColor32& color);
	void setStrokeThickness(float thickness);
	void setFillOpacity(unsigned char opacity);
	void setStrokeOpacity(unsigned char opacity);

	void drawLine(int x1, int y1, int x2, int y2);
	void drawRect(int left, int top, int right, int bottom);
	void clearRect(int left, int top, int right, int bottom);
	void drawRoundRect(int left, int top, int right, int bottom, int radiusX, int radiusY);
	void drawEllipse(PVertex2D center, PVertex2D radii);

	void drawArc(PVertex2D center, PVertex2D radii,
				 PValue startAngle, PValue extentAngle,
				 bool closed = false);

	void drawPolygon(int* x, int* y, unsigned count, bool closed = true, bool antialias=true);

	void drawCharacter(unsigned short character, int x, int y);
    void drawCharacters(int numChars, unsigned short* characters, int* x, int y);
	void drawPath(PGPath* path);
	
	void bitblt(PGSurface* src,
				int sLeft, int sTop, int sRight, int sBottom,
				int dLeft, int dTop, int dRight, int dBottom);
	void bitblt(PGSurface* src, PVertex2D SrcStart, PVertex2D SrcEnd, PVertex2D DstStart, PVertex2D DstEnd);

	void bitblt(PGSurface* src, int dLeft, int dTop, int dRight, int dBottom);

	void _bitblt(PGSurface* src,
				 int sLeft, int sTop, int sRight, int sBottom,
				 int dLeft, int dTop, int dRight, int dBottom);

	void drawPattern(PGSurface* pattern, int phaseX, int phaseY,
					 int dLeft, int dTop, int dRight, int dBottom);

	void drawPattern(PGSurface* pattern, const PMatrix3D& patternMatrix,
					 int dLeft, int dTop, int dRight, int dBottom);
	
	void clipRect(int left, int top, int right, int bottom);
	void addClipRect(int left, int top, int right, int bottom);
	void clipPath(PGPath* Path);    
	void clearClip();
	PRect getClipRect() const;
	float getScale() const;
	
	void setGlobalClipRect(int left, int top, int right, int bottom);

	void setFgColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 0xFF);
	void drawTextA(const char* str, int len, int x, int y, const PGFont* font);

	void concatMatrix(const PMatrix3D& matrix);
	PMatrix3D matrix() const;

private:
	
	PGContext(PContext2D* gc);
	~PGContext();

	PGContextState* currState() const;

	void applyState();
	
private:

	PContext2D* m_gc;

	PGContextState* m_stateStack;	
	int m_currState;

	PGSurface* m_surf;
	PGFont* m_font;

	bool m_stateChanged;

private:

	// Non-Copyable
	PGContext(const PGContext&);
	PGContext& operator=(const PGContext&);	
};

#endif /* PGCONTEXT_H */
