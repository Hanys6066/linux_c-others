#include "graph.h"
/*____________________________________________________________________________*/
/*__________________________________FIGURE____________________________________*/
struct g_figure create_figure(float x, float y, float w, float h);
void rescaleFigure(struct g_figure *fig);
void drawFigure(struct g_figure *fig);
void figureDelete(struct g_figure *fig);
/*____________________Static Function________________*/
static void rescaleFigureFlat(struct g_figure *fig);
static void DrawFigureFlat(struct g_figure *fig);

/*____________________________________________________________________________*/
/*__________________________________PLOT____________________________________*/
int add_plot(struct g_figure *fig, float x , float y, float w, float h);
void set_plotBcColor(struct g_plot *plt, struct Color c);
/*____________________Static Function________________*/
static void rescalePlots(struct g_figure *fig);
static void DrawPlot(struct g_plot *plt, struct g_figure *fig);
static void plotClean(struct g_plot *plot);

/*____________________________________________________________________________*/
/*__________________________________AXIS______________________________________*/
int add_axis(struct g_plot *plt, float x1, float y1, float x2, float y2, G_AXIS_TYPE type);
/*____________________Static Function________________*/
static int updateAxisByPlotSizeChange(struct g_axis *ax, float pltDiag);
static int createAxisPoints(char ***arr, float start, float end, float step, unsigned int digits, unsigned int prev);
static void updateAxisPoints(struct g_axis *ax);
static void updateAxisRange(struct g_plot *plt, struct g_line *line); /*adjust axis ranges according to line points(with respect axis settings) */
static void rescaleAxis(struct g_plot *plt, Boolean resize);
static void drawAxis(struct g_axis *ax);
static void axisClean(struct g_plot *plt);

/*____________________________________________________________________________*/
/*__________________________________LINE______________________________________*/
int add_xyline(struct g_plot *plt, float *xdata, float *ydata, unsigned int n_points);
int setLineData(struct g_plot *plt, unsigned int line_id, float *x_data, float *y_data, unsigned int n_points);
int lineAddData(struct g_plot *plt, unsigned int line_id, float *x_data, float *y_data, unsigned int n_points);
/*____________________Static Function________________*/
static int g_PointsRel(struct g_plot *plt, struct g_line *line);   /*obtain relative coordinace when poits are set*/
static int g_PointsAbs(struct g_plot *plt, struct g_line *line);    /*convert absolute coordinace to absolute(in Axis space)*/
static void DrawXYLine(struct g_line *line, struct g_rec_cord *dSpace);
static void lineClean(struct g_line *line);

/*____________________________________________________________________________*/
/*________________________________OTHERS______________________________________*/
static float g_LineLength(float x1, float y1, float x2, float y2);
static void g_fMinMax(float *arr, unsigned int num, float *min, float *max);
static void textPropotions(float *w, float *h, const char *text, void *font, float scale);

#ifndef PIX_OBJECT_LIB_H
static void drawStrokeText(const char* str, float x, float y, float z,\
                    float scale, float width, float angle, struct Color c);
#endif


/*_____________________________________________________________________________*/
/*___________________________Static variables__________________________________*/

struct Color C_wheel[G_WHEEL_SIZE] = {{0.0f, 0.0f, 0.0f},      /*black*/
                                      {1.0f, 0.0f, 0.0f},      /*red*/
                                      {0.0f, 1.0f, 0.0f},      /*green*/
                                      {0.0f, 0.0f, 1.0f},      /*blue*/
                                      {0.5f, 0.0f, 0.5f},      /*purple*/
                                      {1.0f, 0.5f, 0.0f},      /*orange*/
                                      {0.0f, 0.5f, 1.0f},      /*cyan*/
                                      {0.5f, 0.5f, 0.0f},      /*olive*/
                                      {0.0f, 0.4f, 0.4f},      /*tean*/
                                      {0.6f, 0.3f, 0.0f}       /*brown*/
                                  };

/*____________________________________________________________________________*/
/*______________________________Figure Functions______________________________*/
struct g_figure create_figure(float x, float y, float w, float h)
{
    /*all coordinace are in relative form(0-1)*/
    float w_w = (float)glutGet(GLUT_WINDOW_WIDTH);
    float w_h = (float)glutGet(GLUT_WINDOW_HEIGHT);
    struct g_figure figure = {
        .title = {
            .text = (char *)NULL,
        },
        .plots = (struct g_plot *)NULL,
        .texts = (struct g_title *)NULL,
        .color = G_FIG_COLOR,
        .resize = TRUE,
        .oCord = {
            .x = x,
            .y = y,
            .w = w,
            .h = h
        },
        .tCord = {
            .x = x * w_w,
            .y = y * w_h,
            .w = w * w_w,
            .h = h * w_h
        }
    };
    return figure;
}

void rescaleFigure(struct g_figure *fig)
{
    rescaleFigureFlat(fig);
    rescalePlots(fig);
}

void figureDelete(struct g_figure *fig)
{
    int i;
    for(i = 0; i < fig->num_plots; i++){
        plotClean(&(fig->plots[i]));
    }
    free(fig->plots);
    free(fig->texts);
}

void drawFigure(struct g_figure *fig)
{
    unsigned int i;
    DrawFigureFlat(fig);
    for(i = 0; i < fig->num_plots; i++){
        DrawPlot(&(fig->plots[i]), fig);
    }
}

static void rescaleFigureFlat(struct g_figure *fig)
{
    float w = (float)glutGet(GLUT_WINDOW_WIDTH);
    float h = (float)glutGet(GLUT_WINDOW_HEIGHT);
    fig->tCord.x = fig->oCord.x * w;
    fig->tCord.y = fig->oCord.y * h;
    if(fig->resize == TRUE){
        fig->tCord.w = fig->oCord.w * w;
        fig->tCord.h = fig->oCord.h * h;
    }
}

static void DrawFigureFlat(struct g_figure *fig)
{

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(fig->color.r, fig->color.g, fig->color.b);
    glBegin(GL_POLYGON);
    glVertex2f(fig->tCord.x, fig->tCord.y);
    glVertex2f(fig->tCord.x + fig->tCord.w, fig->tCord.y);
    glVertex2f(fig->tCord.x + fig->tCord.w, fig->tCord.y + fig->tCord.h);
    glVertex2f(fig->tCord.x, fig->tCord.y + fig->tCord.h);
    glEnd();
}

/*____________________________________________________________________________*/
/*________________________________Plot Functions______________________________*/

int add_plot(struct g_figure *fig, float x , float y, float w, float h)
{
    /*all coordinace are in relative form(0-1)*/
    struct g_plot plot = {
        .title = {
            .text = (char *)NULL,
        },
        .id = fig->num_plots,
        .num_lines = 0,
        .num_axis = 0,
        .cWheelId = 0,
        .axis = (struct g_axis *)NULL,
        .lines = (struct g_line *)NULL,
        .bc_color = G_PLOT_COLOR,
        .oCord = (struct g_rec_cord){
            .x = x,
            .y = y,
            .w = w,
            .h = h
        },
        .tCord = (struct g_rec_cord){
            .x = x * fig->tCord.w,
            .y = y * fig->tCord.h,
            .w = w * fig->tCord.w,
            .h = h * fig->tCord.h
        },
        .oDSpace = (struct g_rec_cord){
            .x = G_X_AXIS_X1,
            .y = G_Y_AXIS_Y1,
            .w = G_X_AXIS_X2 - G_X_AXIS_X1,
            .h = G_Y_AXIS_Y2 - G_Y_AXIS_Y1
        },
        .tDSpace = (struct g_rec_cord){
            .x = (G_X_AXIS_X1 * (x * fig->tCord.w)) + fig->tCord.x + (x * fig->tCord.w),
            .y = (G_Y_AXIS_Y1 * (y * fig->tCord.h)) + fig->tCord.y + (y * fig->tCord.w),
            .w = ((G_X_AXIS_X2 - G_X_AXIS_X1) * (w * fig->tCord.w)),
            .h = ((G_Y_AXIS_Y2 - G_Y_AXIS_Y1) * (h * fig->tCord.h))
        }
    };

    /*x-axis*/
    add_axis(&plot, G_X_AXIS_X1, G_X_AXIS_Y1, G_X_AXIS_X2, G_X_AXIS_Y2, G_X_AXIS);
    /*y-axis*/
    add_axis(&plot, G_Y_AXIS_X1, G_Y_AXIS_Y1, G_Y_AXIS_X2, G_Y_AXIS_Y2, G_Y_AXIS);

    /*data space(default same as axis space)*/
    fig->num_plots++;
    if(NULL == (fig->plots = (struct g_plot *)realloc(fig->plots, fig->num_plots * sizeof(struct g_plot)))){
        printf("add_plot(): realloc failed\n");
        return(-1);
    }
    fig->plots[fig->num_plots - 1] = plot;
    return(fig->num_plots - 1);
}

void set_plotBcColor(struct g_plot *plt, struct Color color)
{
    plt->bc_color = color;
}

static void rescalePlots(struct g_figure *fig)
{
    unsigned int i, j;
    for(i = 0; i < fig->num_plots; i++){
        fig->plots[i].tCord.x = fig->plots[i].oCord.x * fig->tCord.w;
        fig->plots[i].tCord.y = fig->plots[i].oCord.y * fig->tCord.h;
        if(fig->resize == TRUE){
            fig->plots[i].tCord.w = fig->plots[i].oCord.w * fig->tCord.w;
            fig->plots[i].tCord.h = fig->plots[i].oCord.h * fig->tCord.h;
            fig->plots[i].tDSpace.w = fig->plots[i].oDSpace.w * fig->plots[i].tCord.w;
            fig->plots[i].tDSpace.h = fig->plots[i].oDSpace.h * fig->plots[i].tCord.h;
        }

        fig->plots[i].tDSpace.x = (fig->plots[i].oDSpace.x * fig->plots[i].tCord.w) + fig->tCord.x + fig->plots[i].tCord.x;
        fig->plots[i].tDSpace.y = (fig->plots[i].oDSpace.y * fig->plots[i].tCord.h) + fig->tCord.y + fig->plots[i].tCord.y;

        rescaleAxis(&(fig->plots[i]), fig->resize);

        for(j = 0; j < fig->plots[i].num_lines; j++){
            g_PointsAbs(&(fig->plots[i]), &(fig->plots[i].lines[j]));
        }
    }
}

static void DrawPlot(struct g_plot *plt, struct g_figure *fig)
{
    int i;
    glLoadIdentity();
    glPushMatrix();
    glTranslatef(fig->tCord.x, fig->tCord.y, 0.0f);
        /*figure space*/
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(plt->bc_color.r, plt->bc_color.g, plt->bc_color.b);
        glBegin(GL_POLYGON);
        glVertex2f(plt->tCord.x, plt->tCord.y);
        glVertex2f(plt->tCord.x + plt->tCord.w, plt->tCord.y);
        glVertex2f(plt->tCord.x + plt->tCord.w, plt->tCord.y + plt->tCord.h);
        glVertex2f(plt->tCord.x, plt->tCord.y + plt->tCord.h);
        glEnd();

        /*draw boundery of plot*/
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_POLYGON);
        glVertex2f(plt->tCord.x, plt->tCord.y);
        glVertex2f(plt->tCord.x + plt->tCord.w, plt->tCord.y);
        glVertex2f(plt->tCord.x + plt->tCord.w, plt->tCord.y + plt->tCord.h);
        glVertex2f(plt->tCord.x, plt->tCord.y + plt->tCord.h);
        glEnd();


        glPushMatrix();
        glTranslatef(plt->tCord.x, plt->tCord.y, 0.0f);
            /*plot space*/
            for(i = 0; i < plt->num_axis; i++){
                drawAxis(&(plt->axis[i]));
            }
        glPopMatrix();
    glPopMatrix();

    glLoadIdentity();
    glPushMatrix();
    glTranslatef(plt->tDSpace.x, plt->tDSpace.y, 0.0f);
        /*data space*/
        for(i = 0; i < plt->num_lines; i++){
            DrawXYLine(&(plt->lines[i]), &(plt->tDSpace));
        }
    glPopMatrix();
}

static void plotClean(struct g_plot *plt)
{
    int i;
    for(i = 0; i < plt->num_lines; i++){
        lineClean(&(plt->lines[i]));
    }
    axisClean(plt);
}

/*____________________________________________________________________________*/
/*________________________________Axis Functions______________________________*/

int add_axis(struct g_plot *plt, float x1, float y1, float x2, float y2, G_AXIS_TYPE type)
{
    struct g_axis *tmp = (struct g_axis *)NULL;
    int i;
    struct g_axis ax = {
        .type = type,
        .oCord = (struct g_rec_cord){
            .x = x1,
            .y = y1,
            .w = x2,
            .h = y2
        },
        .tCord = (struct g_rec_cord){
            .x = x1 * plt->tCord.w,
            .y = y1 * plt->tCord.h,
            .w = x2 * plt->tCord.w,
            .h = y2 * plt->tCord.h
        },
        .label = (char *)NULL,
        .r0 = (type == G_Y_AXIS ? G_AXIS_R0_Y : (type == G_X_AXIS ? G_AXIS_R0_X : 0.0f)),
        .r1 = (type == G_Y_AXIS ? G_AXIS_R1_Y : (type == G_X_AXIS ? G_AXIS_R1_X : 0.0f)),
        .range = G_AXIS_RANGE,
        .numbers = (char **)NULL,
        .fix_lines = FALSE,
        .color = G_AXIS_COLOR,
        .width = G_AXIS_WIDTH
    };

    if(-1 == updateAxisByPlotSizeChange(&ax, g_LineLength(plt->tCord.x, plt->tCord.y, plt->tCord.x + plt->tCord.w,\
                                plt->tCord.y + plt->tCord.h))){
        printf("add_axis(): updateAxisByPlotSizeChange() failed\n");
        return(-1);
    }

    plt->num_axis++;
    if(NULL == (tmp = (struct g_axis *)realloc(plt->axis, sizeof(struct g_axis) * plt->num_axis))){
        printf("add_axis(): realloc() failed");
        return(-1);
    }
    plt->axis = tmp;

    plt->axis[plt->num_axis - 1] = ax;
    return(0);
}

static int updateAxisByPlotSizeChange(struct g_axis *ax, float pltDiag)
{
    float axisLen = g_LineLength(ax->tCord.x, ax->tCord.y, ax->tCord.w, ax->tCord.h);
    float step;
    unsigned int prev = ax->num_n;
    /*default setting of points on axis with respect axis length*/
    if(axisLen > 500.0f){
        ax->num_n = G_AXIS_POINTS_1;
        step = (ax->range[1] - ax->range[0]) / ax->num_n;
        if(-1 == createAxisPoints(&ax->numbers, ax->range[0], ax->range[1], step, G_AXIS_DIG_1, prev)){
            printf("add_axis(): createAxisPoints() failed");
            return(-1);
        }
    }else if(axisLen > 250.0f){
        ax->num_n = G_AXIS_POINTS_2;
        step = (ax->range[1] - ax->range[0]) / ax->num_n;
        if(-1 == createAxisPoints(&ax->numbers, ax->range[0], ax->range[1], step, G_AXIS_DIG_2, prev)){
            printf("add_axis(): createAxisPoints() failed");
            return(-1);
        }
    }else if(axisLen > 100.0f){
        ax->num_n = G_AXIS_POINTS_3;
        step = (ax->range[1] - ax->range[0]) / ax->num_n;
        if(-1 == createAxisPoints(&ax->numbers, ax->range[0], ax->range[1], step, G_AXIS_DIG_3, prev)){
            printf("add_axis(): createAxisPoints() failed");
            return(-1);
        }
    }else{
        ax->num_n = 2;
        step = (ax->range[1] - ax->range[0]) / ax->num_n;
        if(-1 == createAxisPoints(&ax->numbers, ax->range[0], ax->range[1], step, G_AXIS_DIG_3, prev)){
            ax->lines_size = G_AXIS_LINES_SIZE_3;
        }
    }

    /*default setting of texts parameters with respect plot diagonal size*/
    if(pltDiag > 800.0f){
        ax->tx_scale = G_AXIS_TEXT_SCALE_1;
        ax->tx_width = G_AXIS_TEXT_WIDTH_1;
        ax->lines_size = G_AXIS_LINES_SIZE_1;
    }else if(pltDiag > 400.0f){
        ax->tx_scale = G_AXIS_TEXT_SCALE_2;
        ax->tx_width = G_AXIS_TEXT_WIDTH_2;
        ax->lines_size = G_AXIS_LINES_SIZE_2;
    }else{
        ax->tx_scale = G_AXIS_TEXT_SCALE_3;
        ax->tx_width = G_AXIS_TEXT_WIDTH_3;
        ax->lines_size = G_AXIS_LINES_SIZE_3;
    }

    ax->s_x = (ax->tCord.w - ax->tCord.x) / ax->num_n;
    ax->s_y = (ax->tCord.h - ax->tCord.y) / ax->num_n;

    if(ax->s_x != 0.0f){
        ax->tg = atanf(ax->s_y / ax->s_x);
        ax->dir = G_AX_RIGHT;

    }else{
        ax->tg = M_PI / 2.0f;
        ax->dir = G_AX_LEFT;
    }
}


static int createAxisPoints(char ***arr, float start, float end, float step, unsigned int digits, unsigned int prev)
{
    int i, num = (int)floorf((end - start) / step) + 1;/*start number is included*/
    float act_n = start;

    if(num < 0){
        printf("createAxisPoints(): wrong arguments\n");
        return(-1);
    }

    if(*arr != (char **)NULL){/*clear revisous array*/
        for(i = 0; i < prev +1; i++){
            free((*arr)[i]);
            (*arr)[i] = (char *)NULL;
        }
        free(*arr);
        *arr = (char **)NULL;
    }

    if(NULL == (*arr = (char **)malloc(num * sizeof(char *)))){
        printf("createAxisPoints(): malloc() failed\n");
        return(-1);
    }
    for(i = 0; i < num; i++){
        if(NULL == ((*arr)[i] = (char *)malloc(digits + 2))){/*. and \0*/
            printf("createAxisPoints(): malloc() failed\n");
            free(*arr);
            return(-1);
        }
        snprintf((*arr)[i], digits + 2, "%f", act_n);
        act_n += step;
    }
    return(0);
}

static void updateAxisPoints(struct g_axis *ax)
{
    /*update values only*/
    int i;
    float step = (ax->range[1] - ax->range[0]) / ax->num_n;
    float act = ax->range[0];
    for(i = 0; i < ax->num_n + 1; i++){
        snprintf(ax->numbers[i], ax->digits + 2, "%f", act);
        act += step;
    }
}

static void updateAxisRange(struct g_plot *plt, struct g_line *line)
{
    /*if axis is not fix(fix_lines == FALSE) change axis range according to line points
    also set x_range and y_range for given plot*/
    float D_range[2];   /*data range*/
    float Pa_range[2];  /*potential data range*/
    float *data = (float *)data;
    int i, prev;

    if(plt->num_axis > 0){
        for(i = 0; i < plt->num_axis; i++){
            if(plt->axis[i].fix_lines == FALSE){
                if((plt->axis[i].type == G_X_AXIS) || (plt->axis[i].type == G_Y_AXIS)){
                    data = (plt->axis[i].type == G_X_AXIS ? line->x_data: line->y_data);
                    g_fMinMax(data, line->num_points, &(D_range[0]), &(D_range[1]));
                    Pa_range[0] = D_range[0] * (D_range[0] > 0 ? (1.0f - plt->axis[i].r0) : (1.0f + plt->axis[i].r0));
                    Pa_range[1] = D_range[1] * (D_range[1] > 0 ? (1.0f + plt->axis[i].r1) : (1.0f - plt->axis[i].r1));
                    if(plt->num_lines >= 2){/*in case when multiple lines are present in actual plot*/
                        if(plt->axis[i].range[0] > Pa_range[0] * plt->axis[i].r0){
                            plt->axis[i].range[0] = Pa_range[0];
                        }
                        if(plt->axis[i].range[1] < Pa_range[1]){
                            plt->axis[i].range[1] = Pa_range[0];
                        }
                    }else{
                        plt->axis[i].range[0] = Pa_range[0];
                        plt->axis[i].range[1] = Pa_range[1];
                    }
                }
            }
            if(plt->axis[i].type == G_X_AXIS){
                plt->x_range[0] = plt->axis[i].range[0];
                plt->x_range[1] = plt->axis[i].range[1];
            }else if(plt->axis[i].type == G_Y_AXIS){
                plt->y_range[0] = plt->axis[i].range[0];
                plt->y_range[1] = plt->axis[i].range[1];
            }
            updateAxisPoints(&(plt->axis[i]));
        }
    }
}

static void rescaleAxis(struct g_plot *plt, Boolean resize)
{
    int i;
    for(i = 0; i < plt->num_axis; i++){
        plt->axis[i].tCord.x = plt->axis[i].oCord.x * plt->tCord.w;
        plt->axis[i].tCord.y = plt->axis[i].oCord.y * plt->tCord.h;
        if(resize == TRUE){
            plt->axis[i].tCord.w = plt->axis[i].oCord.w * plt->tCord.w;
            plt->axis[i].tCord.h = plt->axis[i].oCord.h * plt->tCord.h;
            if(plt->axis[i].fix_lines == FALSE){
                if(-1 == updateAxisByPlotSizeChange(&(plt->axis[i]), g_LineLength(plt->tCord.x, plt->tCord.y, plt->tCord.x + plt->tCord.w,\
                                            plt->tCord.y + plt->tCord.h))){
                    printf("rescaleAxis(): updateAxisByPlotSizeChange() failed\n");
                }
            }
        }
    }
}

static void drawAxis(struct g_axis *ax)
{
    int i;
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH, GL_NICEST);
    glLineWidth(ax->width);
    glBegin(GL_LINES);
    glColor3f(ax->color.r, ax->color.g, ax->color.b);
    glVertex2f(ax->tCord.x, ax->tCord.y);
    glVertex2f(ax->tCord.w, ax->tCord.h);
    glEnd();

    float dir_x = (ax->dir == G_AX_LEFT ? -1.0f : 1.0f);
    float dir_y = (ax->dir == G_AX_LEFT ? 1.0f : -1.0f);
    float x, y, tx, ty, tw, th;   /*x and y coordinace of text*/
    glPushMatrix();
    /*translate at begining of axis*/
    glTranslatef(ax->tCord.x, ax->tCord.y, 0.0f);
    for(i = 0; i < ax->num_n + 1; i++){
        /*draw actual axis line*/
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glEnable(GL_MULTISAMPLE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_LINE_SMOOTH, GL_NICEST);
        glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        if(ax->s_x != 0 && ax->s_y != 0){
            x = dir_x * ax->lines_size * cosf((M_PI / 2.0f) - ax->tg);
            y = dir_y * ax->lines_size * sinf((M_PI / 2.0f) - ax->tg);
        }else{
            x = dir_x * ax->lines_size * sinf(ax->tg);
            y = dir_y * ax->lines_size * cosf(ax->tg);
        }
        glVertex2f(x, y);
        glEnd();
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_BLEND);

        /*draw actual number*/
        textPropotions(&tw, &th, ax->numbers[i], GLUT_STROKE_ROMAN, ax->tx_scale);
        //printf("width: %f | height: %f\n", tw, th);
        if(ax->s_x == 0){
            tx = x + dir_x * tw;
            ty = -(th / 2.0f);
            drawStrokeText((const char *)ax->numbers[i], tx, ty, 0.0f, ax->tx_scale, ax->tx_width,\
                0.0f, (struct Color){0.0f, 0.0f, 0.0f});
        }else if(ax->s_y == 0){
            tx = -(tw / 2.0f);
            ty = (dir_y * th) + y;
            drawStrokeText((const char *)ax->numbers[i], tx, ty, 0.0f, ax->tx_scale, ax->tx_width,\
                0.0f, (struct Color){0.0f, 0.0f, 0.0f});
        }else{
            tx = (-tw / 2.0f) * cosf(ax->tg) + x;
            ty = dir_y * (th + tw / 2.0f) * sinf(ax->tg) + y;
            drawStrokeText((const char *)ax->numbers[i], tx, ty, 0.0f, ax->tx_scale, ax->tx_width,\
                ax->tg * (90.0f/M_PI), (struct Color){0.0f, 0.0f, 0.0f});
        }

        glTranslatef(ax->s_x, ax->s_y, 0.0f);
    }
    glPopMatrix();
    /*set line width back to 1*/
    glLineWidth(1.0f);
}

static void axisClean(struct g_plot *plt)
{
    int i, j;
    for(i = 0; i < plt->num_axis; i++){
        free(plt->axis[i].label);
        plt->axis[i].label = (char *)NULL;
        for(j = 0; j < plt->axis[i].num_n + 1; j++){
            free(plt->axis[i].numbers[j]);
            plt->axis[i].numbers[j] = (char *)NULL;
        }
        free(plt->axis[i].numbers);
        plt->axis[i].numbers = (char **)NULL;
    }
}

/*____________________________________________________________________________*/
/*________________________________Line Functions______________________________*/

int add_xyline(struct g_plot *plt, float *xdata, float *ydata, unsigned int n_points)
{
    struct g_line *tmp = (struct g_line *)NULL;
    int i;

    struct g_line l = {
        .x_data = (float *)NULL,
        .y_data = (float *)NULL,
        .num_points = n_points,
        .width = G_LINE_WIDTH,
        .pSize = G_LINE_POINTS_SIZE,
        .l_patern = G_FULL_LINE,
        .color = C_wheel[plt->cWheelId],
        .tCord = (struct g_xy_cord *)NULL,
        .oCord = (struct g_xy_cord *)NULL
    };

    plt->num_lines++;
    if(NULL == (tmp = realloc(plt->lines, plt->num_lines * sizeof(struct g_line)))){
        printf("add_xyline(): realloc() falied\n");
        return(-1);
    }
    plt->lines = tmp;
    plt->lines[plt->num_lines - 1] = l;

    /*copy x and y data to line structure, ans perform g_PointsRel() function*/
    if(-1 == setLineData(plt, plt->num_lines - 1, xdata, ydata, n_points)){
        printf("add_xyline(): setLineData failed\n");
        return(-1);
    }

    plt->cWheelId++;
    if(plt->cWheelId >= G_WHEEL_SIZE){
        plt->cWheelId = 0;
    }

    return(plt->num_lines - 1);
}

int setLineData(struct g_plot *plt, unsigned int line_id, float *x_data, float *y_data, unsigned int n_points)
{
    float *tmp = (float *)NULL;
    struct g_line *l = &(plt->lines[line_id]);
    int i;

    if(n_points != 0){
        if(NULL == (tmp = realloc(l->x_data, n_points * sizeof(float)))){
            printf("add_xyline(): malloc() failed\n");
            return(-1);
        }
        l->x_data = tmp;

        if(NULL == (tmp = realloc(l->y_data, n_points * sizeof(float)))){
            printf("add_xyline(): malloc() failed\n");
            return(-1);
        }
        l->y_data = tmp;
        /*copy data*/
        for(i = 0; i < n_points; i++){
            l->y_data[i] = y_data[i];
            l->x_data[i] = x_data[i];
        }
        l->num_points = n_points;
        if(-1 == g_PointsRel(plt, l)){
            printf("add_xyline(): g_PointsRel() failed\n");
            return(-1);
        }
    }
    return(0);
}

int lineAddData(struct g_plot *plt, unsigned int line_id, float *x_data, float *y_data, unsigned int n_points){
    float *tmp = (float *)NULL;
    struct g_line *l = &(plt->lines[line_id]);
    int i = l->num_points;

    l->num_points += n_points;
    if(n_points != 0){
        if(NULL == (tmp = realloc(l->x_data, l->num_points * sizeof(float)))){
            printf("add_xyline(): malloc() failed\n");
            return(-1);
        }
        l->x_data = tmp;

        if(NULL == (tmp = realloc(l->y_data, l->num_points * sizeof(float)))){
            printf("add_xyline(): malloc() failed\n");
            return(-1);
        }
        l->y_data = tmp;
        /*copy data*/
        for(;i < l->num_points; i++){
            l->y_data[i] = *y_data++;
            l->x_data[i] = *x_data++;
        }
        if(-1 == g_PointsRel(plt, l)){
            printf("add_xyline(): g_PointsRel() failed\n");
            return(-1);
        }
    }
    return(0);
}

static int g_PointsRel(struct g_plot *plt, struct g_line *line)
{
    /*call this function whenever the data changes, function fit axis range and
    change axis scale*/
    unsigned int i;
    Boolean x_fix = FALSE, y_fix = FALSE;
    struct g_xy_cord *tmp = (struct g_xy_cord *)NULL;
    float *x_range = (float *)NULL;
    float *y_range = (float *)NULL;

    if(line->num_points > 0){
        updateAxisRange(plt, line);

        if(NULL == (tmp = (struct g_xy_cord *)realloc(line->oCord, line->num_points * sizeof(struct g_xy_cord)))){
            printf("g_PointsRel(): realloc() failed\n");
            return(-1);
        }
        line->oCord = tmp;

        tmp = (struct g_xy_cord*)NULL;
        if(NULL == (tmp = (struct g_xy_cord *)realloc(line->tCord, line->num_points * sizeof(struct g_xy_cord)))){
            printf("g_PointsRel(): realloc() failed\n");
            return(-1);
        }
        line->tCord = tmp;

        /*compute oCord-> relative coordinance*/
        for(i = 0; i < line->num_points; i++){
            line->oCord[i].x = (line->x_data[i] - plt->x_range[0]) / (plt->x_range[1] - plt->x_range[0]);
            line->oCord[i].y = (line->y_data[i] - plt->y_range[0]) / (plt->y_range[1] - plt->y_range[0]);
        }
    }

    if(-1 == g_PointsAbs(plt, line)){
        printf("g_PointsRel(): g_PointsAbs() failed\n");
        return(-1);
    }
    return(0);
}

static int g_PointsAbs(struct g_plot *plt, struct g_line *line)
{
    /*this function convert relative coordinance to absolute(in axis space)
    function is call when window is reshaped or when points are set*/
    int i;
    float x_len, y_len;

    /*set tCord for each point*/
    for(i = 0; i < line->num_points; i++){
        line->tCord[i].x = line->oCord[i].x * plt->tDSpace.w;
        line->tCord[i].y = line->oCord[i].y * plt->tDSpace.h;
    }
    return(0);
}

static void DrawXYLine(struct g_line *line, struct g_rec_cord *dSpace)
{
    int i;
    /*establish crop for axis area ->those points which are outside axis area will not be printed*/
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)(dSpace->x - line->pSize / 2.0f), (int)(dSpace->y - line->pSize / 2.0f),\
                    dSpace->w, dSpace->h);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH, GL_NICEST);

    glLineWidth(line->width);
    glColor3f(line->color.r, line->color.g, line->color.b);
    glBegin(GL_LINE_STRIP);
    for(i = 0; i < line->num_points; i++){
        glVertex2f(line->tCord[i].x, line->tCord[i].y);
    }
    glEnd();

    glPointSize(line->pSize);
    glBegin(GL_POINTS);
    for(i = 0; i < line->num_points; i++){
        glVertex2f(line->tCord[i].x, line->tCord[i].y);
    }
    glEnd();

    /*recover previous settings*/
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    glPointSize(1.0f);
    glLineWidth(1.0f);
}

static void lineClean(struct g_line *line)
{
    free(line->x_data);
    free(line->y_data);
    free(line->oCord);
    free(line->tCord);
    line->x_data = (float *)NULL;
    line->y_data = (float *)NULL;
    line->oCord = (struct g_xy_cord *)NULL;
    line->tCord = (struct g_xy_cord *)NULL;
}

/*____________________________________________________________________________*/
/*________________________________other Functions_____________________________*/
static float g_LineLength(float x1, float y1, float x2, float y2)
{
    return(sqrtf(powf(x2 - x1, 2.0f) + powf(y2 - y1, 2.0f)));
}

static void g_fMinMax(float *arr, unsigned int num, float *min, float *max)
{
    /*find minimun and maximum of given array arr, min will be set as minimun
    and max will be set as maximum*/
    unsigned int i;
    float _max = arr[0];
    float _min = arr[0];

    for(i = 1; i < num; i++){
        if(arr[i] > _max){
            _max = arr[i];
        }
        if(arr[i] < _min){
            _min = arr[i];
        }
    }
    *min = _min;
    *max = _max;
}


static void textPropotions(float *w, float *h, const char *text, void *font, float scale)
{
    int i;
    *w = scale * glutStrokeLengthf(font, text) / 10.f;
    *h = scale * (glutStrokeLengthf(font, "M") / strlen("M")) * 0.18f; /*function glutStrokeHeigth is included only in few version of freeglut*/
}


#ifndef PIX_OBJECT_LIB_H
static void drawStrokeText(const char* str, float x, float y, float z,
                float scale, float width, float angle, struct Color c)
{
    /*draw string in position x, y, z. Text has GLUT_STROKE_ROMAN font,
    which can be scale by parameter scale and rotate of angle angle*/
    size_t i = 0;
    glDisable(GL_POINT_SMOOTH | GL_LINE_SMOOTH | GL_POLYGON_SMOOTH_HINT);
    glPushMatrix();
    glColor3f(c.r, c.g, c.b);
    glTranslatef(x, y + 8.0f, z); /*shift by y_size of font in y direction*/
    glScalef(0.09f * scale, 0.08f * scale, z);
    glRotatef(angle, 1.0f, 0.0f, 0.0f);
    glLineWidth(width);
    glPointSize(1.5f);
    while(str[i] != '\0'){
        glutStrokeCharacter(GLUT_STROKE_ROMAN , str[i]);
        i++;
    }
    glPopMatrix();
    glEnable(GL_POINT_SMOOTH | GL_LINE_SMOOTH | GL_POLYGON_SMOOTH_HINT);
}
#endif
