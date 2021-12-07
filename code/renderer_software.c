////////////////////////////////////
//~ NOTE: Software Renderer
//

inline void
clearToBuffer(PixelBuffer buffer, v4 color)
{
    i32 count = buffer.width*buffer.height;
#if 1
    u32 c = (((u8)(color.r*255.0f) << 16) |
             ((u8)(color.g*255.0f) << 8) |
             ((u8)(color.b*255.0f) << 0));
    
    //u32 *pixel = renderer.pixels;
    for (i32 x = 0; x < count; x++)
        // memcpy is faster, why?
        memcpy((i32*)buffer.pixels + x, &c, 4);
        //*pixel++ = color;
    
    
#elif 0
    //bbggrraa
    
    u8 r = (u8)(color.r*255.0f);
    u8 g = (u8)(color.g*255.0f);
    u8 b = (u8)(color.b*255.0f);
    
    __m128i c = { b, g, r, 0xff,
        b, g, r, 0xff,
        b, g, r, 0xff,
        b, g, r, 0xff,
    };
    
    //__m128i c = { (u8)color.b, (u8)color.g, (u8)color.r, 0xff,
    //(u8)color.b, (u8)color.g, (u8)color.r, 0xff,
    //(u8)color.b, (u8)color.g, (u8)color.r, 0xff,
    //(u8)color.b, (u8)color.g, (u8)color.r, 0xff,
    //};
    
    //__m128i pixelQuad = {
	//0xff, 0xff, 0xff, 0xff,
    //0xff, 0xff, 0xff, 0xff,
    //0xff, 0xff, 0xff, 0xff,
    //0xff, 0xff, 0xff, 0xff,
    //};
    
    for (int x = 0; x < count; x += 4)
    {
        _mm_store_si128((__m128i *)(state->pixels + x), c);
        
        //memcpy((Pixel32 *)gBackBuffer.Memory + x, &pixelQuad, 4);
    }
#endif
}

// converts HSB(=HSV) color to RGB color
v4 colorHSB(v4 colorHSB)
{
	if (colorHSB.r < 0 || colorHSB.r > 255)
		colorHSB.r = 0;
	if (colorHSB.g < 0 || colorHSB.g > 255)
		colorHSB.g = 0;
	if (colorHSB.b < 0 || colorHSB.b > 255)
		colorHSB.b = 0;
    
	f32 r = 0, g = 0, b = 0, h, s, v;
	h = colorHSB.r / 256.0f;
	s = colorHSB.g / 256.0f;
	v = colorHSB.b / 256.0f;
    
	if (s == 0.0f)
		r = g = b = v;
	else
	{
		f32 f, p, q, t;
		i32 i;
		h *= 6.0f;
		//TODO: CHeck This
		//i = i32(floor(h));
		i = (i32)h;
		f = h - i;
		p = v * (1.0f - s);
		q = v * (1.0f - (s * f));
		t = v * (1.0f - (s * (1.0f - f)));
        
		switch (i)
		{
            case 0:
                r = v; g = t; b = p;
                break;
            case 1:
                r = q; g = v; b = p;
                break;
            case 2:
                r = p; g = v; b = t;
                break;
            case 3:
                r = p; g = q; b = v;
                break;
            case 4:
                r = t; g = p; b = v;
                break;
            case 5:
                r = v; g = p; b = q;
                break;
		}
	}
    
	v4 col;
	col.r = r * 255.f;
	col.g = g * 255.f;
	col.b = b * 255.f;
	col.a = 255.f;
	return col;
}

// converts HSL color to RGB color
v4 colorHSL(v4 colorHSL)
{
	f32 r, g, b, h, s, l;
	f32 tempCol1, tempCol2, tempRed, tempGreen, tempBlue;
	h = (f32)colorHSL.r / 256.0f;
	s = (f32)colorHSL.g / 256.0f;
	l = (f32)colorHSL.b / 256.0f;
    
	if (s == 0)
		r = g = b = l;
	else
	{
		if (l < 0.5)
			tempCol2 = l * (1 + s);
		else
			tempCol2 = (l + s) - (l * s);
        
		tempCol1 = 2 * l - tempCol2;
		tempRed = h + 1.0f / 3.0f;
        
		if (tempRed > 1)
			tempRed--;
        
		tempGreen = h;
		tempBlue = h - 1.0f / 3.0f;
        
		if (tempBlue < 0)
			tempBlue++;
        
		//red
		if (tempRed < 1.f / 6.f)
			r = tempCol1 + (tempCol2 - tempCol1) * 6.f * tempRed;
		else if (tempRed < 0.5f)
			r = tempCol2;
		else if (tempRed < 2.f / 3.f)
			r = tempCol1 + (tempCol2 - tempCol1) * ((2.f / 3.f) - tempRed) * 6.f;
		else
			r = tempCol1;
        
		//green
		if (tempGreen < 1.f / 6.f)
			g = tempCol1 + (tempCol2 - tempCol1) * 6.f * tempGreen;
		else if (tempGreen < 0.5)
			g = tempCol2;
		else if (tempGreen < 2.f / 3.f)
			g = tempCol1 + (tempCol2 - tempCol1) * ((2.f / 3.f) - tempGreen) * 6.f;
		else
			g = tempCol1;
        
		//blue
		if (tempBlue < 1.0 / 6.0)
			b = tempCol1 + (tempCol2 - tempCol1) * 6.f * tempBlue;
		else if (tempBlue < 0.5)
			b = tempCol2;
		else if (tempBlue < 2.f / 3.f)
			b = tempCol1 + (tempCol2 - tempCol1) * ((2.f / 3.f) - tempBlue) * 6.f;
		else
			b = tempCol1;
	}
    
	v4 col;
	col.r = r * 255.f;
	col.g = g * 255.f;
	col.b = b * 255.f;
	col.a = 255.f;
	return col;
}

//~ NOTE: These functions are dependant on the global state variable

inline void
clear(v4 color)
{
    clearToBuffer(state->backBuffer, color);
}

// NOTE: alpha blending, internal function
internal inline void
drawBlendedPixel(i32 x, i32 y, v4 color)
{
    if (color.a >= 1.0f)
    {
        // alpha blend not used
        u32 c = ((u32Round(color.r*255.0f) << 16) |
                 (u32Round(color.g*255.0f) << 8) |
                 (u32Round(color.b*255.0f) << 0));
        
        state->pixels[y*state->windowWidth + x] = c;
        return;
    }
    
    // get a number between 0.0-1.0 for the alpha channel
    f32 a = color.a;
    f32 sR = color.r*255.f;
    f32 sG = color.g*255.f;
    f32 sB = color.b*255.f;
    
    u32 tempcol = state->pixels[y*state->windowWidth + x];
    
    f32 dR = (f32)((tempcol >> 16) & 0xff);
    f32 dB = (f32)((tempcol >> 8) & 0xff);
    f32 dG = (f32)((tempcol >> 8) & 0xff);
    
    // linear blend
    f32 r = (1.f - a)*dR + a*sR;
    f32 g = (1.f - a)*dG + a*sG;
    f32 b = (1.f - a)*dB + a*sB;
    
    state->pixels[y*state->windowWidth + x] = (u32Round(r) << 16) | (u32Round(g) << 8) | (u32Round(b) << 0);
}

inline void
drawPixel(f32 pX, f32 pY, v4 color)
{
    i32 x = i32Round(pX);
    i32 y = i32Round(pY);
    
    if ((x < 0) || (x > state->windowWidth - 1) || (y < 0) || (y > state->windowHeight - 1))
        return;
    
    if (color.a < 1.0f)
    {
        // alpha blending on
        drawBlendedPixel(x, y, color);
    } 
    else
    {
        u32 c = (((u8)(color.r*255.f) << 16) |
                 ((u8)(color.g*255.f) << 8) |
                 ((u8)(color.b*255.f) << 0));
        
        state->pixels[y * state->windowWidth + x] = c;
    }
}

v4 getPixel(f32 x, f32 y)
{
    u32 col = state->pixels[(i32)y * state->windowWidth + (i32)x];
    
    f32 r = (f32)((col >> 16) & 0xff) / 255.f;
    f32 g = (f32)((col >> 8) & 0xff) / 255.f;
    f32 b = (f32)((col >> 0) & 0xff) / 255.f;
    v4 result = { r, g, b, 1.f };
    return result;
}

// draws a line with Breshenam's algorithm
// Bresenham's algorithm starts by plotting a pixel at the first coordinate of the line
// (x1, y1), and to x+1, it takes the difference of the y component of the line to the
// two possible y coordinates, and uses the y coordinate where the error is the smaller,
// and repeats this for every pixel.
void drawLine(f32 inX1, f32 inY1, f32 inX2, f32 inY2, v4 color)
{
    // TODO: Round?
    i32 x1 = i32Round(inX1);
    i32 y1 = i32Round(inY1);
    i32 x2 = i32Round(inX2);
    i32 y2 = i32Round(inY2);
    
    b32 step = abs(x2 - x1) < abs(y2 - y1);
    
    // rotate the line
    if (step)
    {
        swap(x1, y1);
        swap(x2, y2);
    }
    
    // x1 have to be on the left of x2
    // if it's to the right: swap it
    if (x2 < x1)
    {
        swap(x1, x2);
        swap(y1, y2);
    }
    
    // The error variable gives us the distance to the best straight line from our current (x, y)
    // pixel. Each time error is greater than one pixel, we increase (or decrease) y by one, and
    // decrease the error by one as well.
    f32 error = 0.0;
    
    // line slope
    f32 slope = (f32)abs(y2 - y1) / (x2 - x1);
    
    // starting point
    i32 y = y1;
    
    i32 ystep = (y2 > y1 ? 1 : -1);
    
    for (i32 i = x1; i < x2; i += 1)
    {
        if (step) 
            drawPixel((f32)y, (f32)i, color);
        else 
            drawPixel((f32)i, (f32)y, color);
        
        error += slope;
        
        if (error >= 0.5)
        {
            y += ystep;
            error -= 1.0;
        }
    }
}

void drawRect(f32 x, f32 y, f32 width, f32 height, v4 color)
{
    drawLine(x, y, x + width, y, color);					 // top
    drawLine(x, y, x, y + height, color);					// left
    drawLine(x, y + height, x + width, y + height, color);   // bottom
    drawLine(x + width, y, x + width, y + height, color);	// right
}

void drawFilledRectToBuffer(PixelBuffer *buffer, f32 pX, f32 pY, f32 width, f32 height, v4 color)
{
    
    i32 minX = (i32)pX;//i32Round(pX);
    i32 minY = (i32)pY;//i32Round(pY);
    i32 maxX = (i32)(pX+width);//i32Round(pX+width);
    i32 maxY = (i32)(pY+height);//i32Round(pY+height);
    
    if(minX < 0)
        minX = 0;
    
    if(minY < 0)
        minY = 0;
    
    // TODO: check if it should be -1
    if(maxX > buffer->width-1)
        maxX = buffer->width-1;
    
    if(maxY > buffer->height-1)
        maxY = buffer->height-1;
    
#if 1
    // get the pixel pointer so it is pointing to the top left corner of the rectangle
    u8 *row = ((u8 *)buffer->pixels + minX*buffer->bytesPerPixel + minY*buffer->pitch);
    for(i32 y = minY; y < maxY; ++y)
    {
        u32 *pixel = (u32 *)row;
        for(i32 x = minX; x < maxX; ++x)
        {            
            drawBlendedPixel(x, y, color);
            //*pixel++ = c;
        }
        
        row += buffer->pitch;
    }
 #elif 0
    for (i32 y = minY; y < maxY; y++)
    {
        u32 *pixel = (u32*)state.pixels + minX + state.backBufferWidth * y;
        for (i32 x = minX; x < maxX; x++)
        {
            if (color.a < 1.0f)
            {
                // alpha blending used
                drawBlendedPixel(x, y, color);
            }
            else
            {
                *pixel++ = c;
            }
        }
    }
#endif
}


void drawFilledRect(f32 pX, f32 pY, f32 width, f32 height, v4 color)
{
    drawFilledRectToBuffer(&state->backBuffer, pX, pY, width, height, color);
}

void drawCircle(f32 inX, f32 inY, f32 inRadius, v4 color)
{
    // TODO: Round?
    i32 xc = (i32)inX;
    i32 yc = (i32)inY;
    i32 radius = (i32)inRadius;
    
    if (xc + radius < 0 || xc - radius >= state->windowWidth || yc + radius < 0 || yc - radius >= state->windowHeight) 
        return;
    
    i32 x1 = radius;
    i32 y1 = 0;
    i32 error = 0;
    
    while (x1 >= y1)
    {
        drawPixel((f32)(xc + x1), (f32)(yc + y1), color);
        drawPixel((f32)(xc + y1), (f32)(yc + x1), color);
        drawPixel((f32)(xc - y1), (f32)(yc + x1), color);
        drawPixel((f32)(xc - x1), (f32)(yc + y1), color);
        drawPixel((f32)(xc - x1), (f32)(yc - y1), color);
        drawPixel((f32)(xc - y1), (f32)(yc - x1), color);
        drawPixel((f32)(xc + y1), (f32)(yc - x1), color);
        drawPixel((f32)(xc + x1), (f32)(yc - y1), color);
        
        y1 += 1;
        error += 1 + 2 * y1;
        if (2 * (error - x1) + 1 > 0)
        {
            x1 -= 1;
            error += 1 - 2 * x1;
        }
    }
}

internal inline void
drawHorizontalLine(i32 x1, i32 x2, i32 y, v4 color)
{
    if (x2 < x1)
    {
        swap(x1, x2);
    }
    
    for (; x1 <= x2; x1++)
        drawPixel((f32)x1, (f32)y, color);
}

void drawFilledCircle(f32 inX, f32 inY, f32 inRadius, v4 color)
{
    
    i32 xc = i32Round(inX);
    i32 yc = i32Round(inY);
    i32 radius = i32Round(inRadius);
    
    if (xc + radius < 0 || xc - radius >= state->windowWidth || yc + radius < 0 || yc - radius >= state->windowHeight) 
        return;
    
    i32 p = 3 - (radius << 1);
    // previous values: to avoid drawing horizontal lines multiple times  (ensure initial value is outside the range)
    i32 pb = yc + radius + 1, pd = yc + radius + 1; 
    i32 a, b, c, d, e, f, g, h;
    i32 x = 0;
    i32 y = radius;
    while (x <= y)
    {
        a = xc + x;
        b = yc + y;
        c = xc - x;
        d = yc - y;
        e = xc + y;
        f = yc + x;
        g = xc - y;
        h = yc - x;
        if (b != pb)
        {
            drawHorizontalLine(a, c, b, color);
        }
        if (d != pd)
        {
            drawHorizontalLine(a, c, d, color);
        }
        if (f != b)
        {
            drawHorizontalLine(e, g, f, color);
        }
        if (h != d && h != f)
        {
            drawHorizontalLine(e, g, h, color);
        }
        
        pb = b;
        pd = d;
        if (p < 0) 
        {
            p += (x++ << 2) + 6;
        }
        else
        {
            p += ((x++ - y--) << 2) + 10;
        }
    }
}

void drawFilledCircle2(f32 x, f32 y, f32 r, v4 color)
{
    
    i32 radius = i32Round(r);
    i32 r2 = radius * radius;
    i32 area = r2 << 2;
    i32 rr = radius << 1;
    
    for (i32 i = 0; i < area; i++) {
        i32 tx = (i % rr) - radius;
        i32 ty = (i / rr) - radius;
        
        if (tx * tx + ty * ty <= r2)
            drawPixel((f32)(x + tx), (f32)(y + ty), color);
    }
}

inline void
drawTriangle(f32 x1, f32 y1, f32 x2, f32 y2,f32 x3, f32 y3, v4 color)
{
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x3, y3, color);
    drawLine(x3, y3, x1, y1, color);
}


// Filled triangel using the Bresenham algorithm
// https://www.avrfreaks.net/sites/default/files/triangles.c
// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void drawFilledTriangle(f32 inX1, f32 inY1, f32 inX2, f32 inY2, f32 inX3, f32 inY3, v4 col)
{
    // TODO: round?
    i32 x1 = (i32)(inX1);
    i32 x2 = (i32)(inX2);
    i32 x3 = (i32)(inX3);
    i32 y1 = (i32)(inY1);
    i32 y2 = (i32)(inY2);
    i32 y3 = (i32)(inY3);
    
    
    int t1x, t2x, y, minx, maxx, t1xp, t2xp;
    b32 changed1 = false;
    b32 changed2 = false;
    int signx1, signx2, dx1, dy1, dx2, dy2;
    int e1, e2;
    // Sort vertices
    if (y1 > y2) { swap(y1, y2); swap(x1, x2); }
    if (y1 > y3) { swap(y1, y3); swap(x1, x3); }
    if (y2 > y3) { swap(y2, y3); swap(x2, x3); }
    
    t1x = t2x = x1; y = y1;   // Starting points
    dx1 = (int)(x2 - x1);
    if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
    else signx1 = 1;
    dy1 = (int)(y2 - y1);
    
    dx2 = (int)(x3 - x1);
    if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
    else signx2 = 1;
    dy2 = (int)(y3 - y1);
    
    if (dy1 > dx1) { swap(dx1, dy1); changed1 = true; }
    if (dy2 > dx2) { swap(dy2, dx2); changed2 = true; }
    
    e2 = (int)(dx2 >> 1);
    // Flat top, just process the second half
    if (y1 == y2) goto next;
    e1 = (int)(dx1 >> 1);
    
    for (int i = 0; i < dx1;) {
        t1xp = 0; t2xp = 0;
        if (t1x < t2x) { minx = t1x; maxx = t2x; }
        else { minx = t2x; maxx = t1x; }
        // process first line until y value is about to change
        while (i < dx1) {
            i++;
            e1 += dy1;
            while (e1 >= dx1) {
                e1 -= dx1;
                if (changed1) t1xp = signx1;//t1x += signx1;
                else          goto next1;
            }
            if (changed1) break;
            else t1x += signx1;
        }
        // Move line
        next1:
        // process second line until y value is about to change
        while (1) {
            e2 += dy2;
            while (e2 >= dx2) {
                e2 -= dx2;
                if (changed2) t2xp = signx2;//t2x += signx2;
                else          goto next2;
            }
            if (changed2)     break;
            else              t2x += signx2;
        }
        next2:
        if (minx > t1x) minx = t1x;
        if (minx > t2x) minx = t2x;
        if (maxx < t1x) maxx = t1x;
        if (maxx < t2x) maxx = t2x;
        drawHorizontalLine(minx, maxx, y, col);    // Draw line from min to max points found on the y
        //drawLine(minx, y, maxx, y, col);
        // Now increase y
        if (!changed1) t1x += signx1;
        t1x += t1xp;
        if (!changed2) t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y == y2) break;
        
    }
    next:
    // Second half
    dx1 = (int)(x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
    else signx1 = 1;
    dy1 = (int)(y3 - y2);
    t1x = x2;
    
    if (dy1 > dx1) {   // swap values
        swap(dy1, dx1);
        changed1 = true;
    }
    else changed1 = false;
    
    e1 = (int)(dx1 >> 1);
    
    for (int i = 0; i <= dx1; i++) {
        t1xp = 0; t2xp = 0;
        if (t1x < t2x) { minx = t1x; maxx = t2x; }
        else { minx = t2x; maxx = t1x; }
        // process first line until y value is about to change
        while (i < dx1) {
            e1 += dy1;
            while (e1 >= dx1) {
                e1 -= dx1;
                if (changed1) { t1xp = signx1; break; }//t1x += signx1;
                else          goto next3;
            }
            if (changed1) break;
            else   	   	  t1x += signx1;
            if (i < dx1) i++;
        }
        next3:
        // process second line until y value is about to change
        while (t2x != x3) {
            e2 += dy2;
            while (e2 >= dx2) {
                e2 -= dx2;
                if (changed2) t2xp = signx2;
                else          goto next4;
            }
            if (changed2)     break;
            else              t2x += signx2;
        }
        next4:
        
        if (minx > t1x) minx = t1x;
        if (minx > t2x) minx = t2x;
        if (maxx < t1x) maxx = t1x;
        if (maxx < t2x) maxx = t2x;
        drawHorizontalLine(minx, maxx, y, col);
        //drawLine(minx, y, maxx, y, col);
        if (!changed1) t1x += signx1;
        t1x += t1xp;
        if (!changed2) t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y > y3) return;
    }
}


void drawSoftSprite(u8 *spriteData, i32 x, i32 y, i32 width, i32 height, i32 col)
{
    if ((x<0) || (x>state->windowWidth - width) || (y<0) || (y>state->windowHeight - height)) return;
    
    //i32 c = col.r << 16 | col.g << 8 | col.b | 0xff000000;
    //u8 *sprite = spriteMap[id];
    
    i32 index=0;
    i32 yOffset = y * state->windowWidth + x;
    for (i32 i = 0; i < height; i++)
    {
        for (i32 j = 0; j < width; j++)
        {
            if (spriteData[index++])
            {
                (state->pixels)[yOffset + j] = col;
            }
        }
        yOffset += state->windowWidth;
    }
}


typedef struct
{
    i32 width;
    i32 height;
    u32 *pixels;
} Texture;

#if 0
Texture createTexture(i32 width, i32 height)
{
    Texture tex;
    tex.width = width;
    tex.height = height;
    // TODO: use memory arena
    //tex.pixels = VirtualAlloc(0, sizeof(u32) * tex.width * tex.height,
    //MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    tex.pixels = memoryPushSize(&memoryArena, width*height*4);
    return tex;
}
#endif

void drawTexture(f32 inX, f32 inY, Texture *sprite)
{
    // round
    i32 minX = i32Round(inX);
    i32 minY = i32Round(inY);
    i32 maxX = i32Round(inX + (f32)sprite->width);
    i32 maxY = i32Round(inY + (f32)sprite->height);
    
    // clip
    if (minX < 0)
        minX = 0;
    
    if (minY < 0)
        minY = 0;
    
    if (maxX > state->windowWidth)
        maxX = state->windowWidth;
    
    if (maxY > state->windowHeight)
        maxY = state->windowHeight;
    
    // because the bitmap is stored bottom up we have to start at the last row and move up
    // go to last row
    u32 *sourceRow = sprite->pixels + sprite->width * (sprite->height - 1);
    
    //point to top left corner of rectangle
    u8 *destRow = ((u8 *)state->pixels +
                   minX * state->backBuffer.bytesPerPixel +
                   minY * state->backBuffer.pitch);
    
    for (i32 y = minY; y < maxY; ++y)
    {
        u32 *dest = (u32 *)destRow;
        u32 *source = sourceRow;
        for (i32 x = minX; x < maxX; ++x)
        {
            //get the percentage of the aplha value, get a number between 0.0-1.0
            f32 a = (f32)((*source >> 24) & 0xff) / 255.0f;
            f32 sR = (f32)((*source >> 16) & 0xff);
            f32 sG = (f32)((*source >> 8) & 0xff);
            f32 sB = (f32)((*source >> 0) & 0xff);
            
            f32 dR = (f32)((*dest >> 16) & 0xff);
            f32 dG = (f32)((*dest >> 8) & 0xff);
            f32 dB = (f32)((*dest >> 0) & 0xff);
            
            /*  linear blend
                t could be thought as, some procentage of,
                it's how much alpha we want
                A and B are the two colors
                Color = A + t(B-A)
                        A + tB - tA
                        A - tA + tB
                        (1-t)A + tB
            */
            
            f32 r = (1.0f - a) * dR + a * sR;
            f32 g = (1.0f - a) * dG + a * sG;
            f32 b = (1.0f - a) * dB + a * sB;
            
            // 0.5 is for rounding
            *dest = (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            
            // alpha test -> when the alpha is bound to some threshold
            // don't copy pixels with alpha below 128
            //if((*source >> 24) > 128)
            //	*dest = *source;
            
            dest++;
            source++;
        }
        destRow += state->backBuffer.pitch;
        sourceRow -= sprite->width;
    }
}

// pack the struct tight on 1byte boundaries = no padding
#pragma pack(push, 1)
typedef struct 
{
    u16 fileType;        /* File type, always 4D42h ("BM") */
    u32 fileSize;        /* Size of the file in bytes */
    u16 reserved1;       /* Always 0 */
    u16 reserved2;       /* Always 0 */
    u32 bitmapOffset;    /* Starting position of image data in bytes */
    u32 size;            /* Size of this header in bytes */
    i32 width;           /* Image width in pixels */
    i32 height;          /* Image height in pixels */
    u16 planes;          /* Number of color planes */
    u16 bitsPerPixel;    /* Number of bits per pixel */
    u32 compression;
    u32 sizeOfBitmap;
    i32 horzResolution;
    i32 vertResolution;
    u32 colorsUsed;
    u32 colorsImportant;
    
    u32 redMask;
    u32 greenMask;
    u32 blueMask;
} BitmapHeader;
#pragma pack(pop)

typedef struct 
{
    b32 found;
    u32 index;
} BitScanResult;

// scan from right to left and returns the position of the first bit set
// testShift = how many steps to shift
internal BitScanResult findLeastSignificantSetBit(u32 value)
{
    BitScanResult result = {0};
//#if COMPILER_MSVC
//    result.found = _BitScanForward((unsigned long *)&result.index, value);
//#else
    for (u32 testShift = 0; testShift < 32; ++testShift)
    {
        if (value & (1 << testShift))
        {
            result.index = testShift;
            result.found = true;
            break;
        }
    }
    
//#endif
    
    return result;
}

Texture loadBitmap(char *filename)
{
    Texture result = {0};
    
    //NOTE: byte order in memory is AA BB GG RR, bottom up
    // when it's loaded into memory on a little endian machine -> 0xRRGGBBAA
    File contents = state->memory->loadFile(filename);
    if (contents.data)
    {
        // to list the file contents in bytes put (uint8*)readResult.contents,64 in MSVC debugger 
        // file size ->(uint32 *)(((uint8*)readResult.contents) + 2)
        BitmapHeader *header = (BitmapHeader *)contents.data;
        //start position of bitmap = beginning of file + bitmapOffset
        u32 *pixels = (u32 *)((u8 *)contents.data + header->bitmapOffset);
        result.pixels = pixels;
        result.width = header->width;
        result.height = header->height;
        
        assert(header->compression == 3);
        
        i32 redMask = header->redMask;
        i32 greenMask = header->greenMask;
        i32 blueMask = header->blueMask;
        i32 alphaMask = ~(redMask | greenMask | blueMask);
        
        // find how much we have to shift
        BitScanResult redShift = findLeastSignificantSetBit(redMask);
        BitScanResult greenShift = findLeastSignificantSetBit(greenMask);
        BitScanResult blueShift = findLeastSignificantSetBit(blueMask);
        BitScanResult alphaShift = findLeastSignificantSetBit(alphaMask);
        
        assert(redShift.found);
        assert(greenShift.found);
        assert(blueShift.found);
        assert(alphaShift.found);
        
        // change 0xRRGGBBAA to 0xAARRGGBB
        u32 *sourceDest = pixels;
        for (i32 y = 0; y < header->height; ++y)
        {
            for (i32 x = 0; x < header->width; ++x)
            {
                u32 c = *sourceDest;
                *sourceDest++ = ((((c >> alphaShift.index) & 0xff) << 24) |
                                 (((c >> redShift.index) & 0xff) << 16) |
                                 (((c >> greenShift.index) & 0xff) << 8) |
                                 (((c >> blueShift.index) & 0xff) << 0));
            }
        }
    }
    
    return result;
}

void textureFree(Texture *texture)
{
    state->memory->freeMemory(texture->pixels);
}