package com.example.android.basicgesturedetect;

/**
 * Created by boris on 7/19/2015.
 */
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import java.util.ArrayList;
import com.example.android.common.logger.Log;
import com.example.android.common.logger.LogFragment;

public class MainDrawingView extends View {

    private Paint paint = new Paint();
    private Path path = new Path();

    public ArrayList<Integer[]> pointList;
    //path optimization stufsf(as in simplification)
    private int radious = 50; // we're gonna try and reduce the number of points so that the printer doesn't have to work so hard, the bigger the radious the less accurate we'll be
    private int previousX;
    private int previousY;
    private boolean newDrawing = true;
    public static final String TAG = "MainDrawingView";

    public MainDrawingView(Context context, AttributeSet attrs) {
        super(context, attrs);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(5f);
        paint.setColor(Color.BLACK);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeJoin(Paint.Join.ROUND);

        pointList = new ArrayList<Integer[]>();
    }
    @Override
    protected void onDraw(Canvas canvas) {
        canvas.drawPath(path, paint);
    }
    @Override
    public boolean onTouchEvent(MotionEvent event){
        //get touch event coordinates
        //try without scaling
        int scaleFactor = 1;
        int drawingHeight = 1050;  //trial and error
        int eventX = Math.round(event.getX()/ scaleFactor);
        int eventY = Math.round(event.getY()/ scaleFactor);
        int tempIntY;
        switch (event.getAction()){
            case MotionEvent.ACTION_DOWN:
                //new starting point
                //Log.i(TAG, "Action down - point with coordinates - [" + Float.toString(eventX) + "," + Float.toString(eventY) + " ]");
                path.moveTo(eventX,eventY);
                previousX = eventX;
                previousY = eventY;
                tempIntY = drawingHeight - eventY;
                if (tempIntY < 0) tempIntY = 0;
                Integer[] pointArr ={eventX,tempIntY};
                pointList.add(pointArr);
                Log.i(TAG, formatPointArray());
                return true;
            case MotionEvent.ACTION_MOVE:
                //connect the point
                //Log.i(TAG, "Action move - point with coordinates - [" + Float.toString(eventX) + "," + Float.toString(eventY) + " ]");
                Double pointDist =  Math.sqrt(Math.pow(eventX - previousX,2) + Math.pow(eventY - previousY, 2));
                //Log.i(TAG, "distance is - " + Double.toString(pointDist));
                //take the point into account only if it's far enough from the previous one
                if (pointDist > radious) {
                    path.lineTo(eventX, eventY);
                    previousX = eventX;
                    previousY = eventY;
                    tempIntY = drawingHeight - eventY;
                    if (tempIntY < 0) tempIntY = 0;
                    Integer[] pointArr1 ={eventX,tempIntY};
                    pointList.add(pointArr1);
                    //repaints and calls redraw
                    invalidate();
                    return true;
                }
                return false;
            default:
                return false;
        }
    }
    public void pressedPrint() {
        //when printing we do not want to reprint the drawn points
        pointList.clear();
    }
    public void clearCanvasDrawing() {
        path.reset();
        pointList.clear();
        this.invalidate();
        newDrawing = true;
    }
    public String formatPointArray() {
        String pointsStr = "http://192.168.240.1/arduino/points/";
        if (newDrawing) {
            pointsStr += "0,0/";
            newDrawing = false;
        }
        int numOfPoints = pointList.size();
        for (int i = 0; i < numOfPoints; i++) {
            pointsStr += Integer.toString(pointList.get(i)[0]) + "," + Integer.toString(pointList.get(i)[1]) + "/";
        }
        pointsStr += "-8";
        return pointsStr;
    }
}