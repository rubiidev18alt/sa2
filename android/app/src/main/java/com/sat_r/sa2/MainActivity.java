package com.sat_r.sa2;

import android.app.Activity;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.widget.FrameLayout;
import android.content.Context;
import android.util.SparseIntArray;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {
    private static native void nativeSetKey(int button, boolean down);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        hideSystemUi();

        View content = findViewById(android.R.id.content);
        if (content instanceof ViewGroup) {
            TouchOverlay overlay = new TouchOverlay(this);
            ((ViewGroup) content).addView(overlay, new ViewGroup.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT));
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) hideSystemUi();
    }

    private void hideSystemUi() {
        Window window = getWindow();
        if (android.os.Build.VERSION.SDK_INT >= 30) {
            WindowInsetsController controller = window.getInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            window.getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_FULLSCREEN |
                    View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                    View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        }
    }

    private static final class TouchOverlay extends View {
        private static final int NONE = -1;
        private static final int A = 0, B = 1, START = 2, SELECT = 3, L = 4, R = 5;
        private static final int UP = 6, DOWN = 7, LEFT = 8, RIGHT = 9;

        private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final SparseIntArray pointerControls = new SparseIntArray();
        private final boolean[] pressed = new boolean[10];

        TouchOverlay(Context context) {
            super(context);
            setFocusable(false);
            setWillNotDraw(false);
            paint.setTypeface(android.graphics.Typeface.create("sans", android.graphics.Typeface.BOLD));
        }

        private float sx(float logicalX) { return logicalX * getWidth() / 426.0f; }
        private float sy(float logicalY) { return logicalY * getHeight() / 240.0f; }

        private boolean circle(float x, float y, float cx, float cy, float r) {
            float dx = x - sx(cx), dy = y - sy(cy);
            return dx * dx + dy * dy <= Math.pow(sx(r), 2);
        }

        private boolean rect(float x, float y, float cx, float cy, float hw, float hh) {
            return x >= sx(cx - hw) && x <= sx(cx + hw) && y >= sy(cy - hh) && y <= sy(cy + hh);
        }

        private int controlAt(float x, float y) {
            if (circle(x, y, 54, 184, 46)) {
                float dx = x - sx(54), dy = y - sy(184);
                if (Math.abs(dx) < sx(10) && Math.abs(dy) < sy(10)) return NONE;
                if (Math.abs(dx) >= Math.abs(dy)) return dx < 0 ? LEFT : RIGHT;
                return dy < 0 ? UP : DOWN;
            }
            if (circle(x, y, 366, 181, 29)) return A;
            if (circle(x, y, 320, 202, 25)) return B;
            if (rect(x, y, 213, 225, 29, 11)) return START;
            if (rect(x, y, 151, 225, 29, 11)) return SELECT;
            if (rect(x, y, 96, 28, 22, 16)) return L;
            if (rect(x, y, 330, 28, 22, 16)) return R;
            return NONE;
        }

        private void setPressed(int control, boolean down) {
            if (control == NONE) return;
            if (pressed[control] == down) return;
            pressed[control] = down;
            nativeSetKey(control, down);
            invalidate();
        }

        private void releasePointer(int pointerId) {
            int control = pointerControls.get(pointerId, NONE);
            if (control != NONE) setPressed(control, false);
            pointerControls.delete(pointerId);
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            final int action = event.getActionMasked();
            final int index = event.getActionIndex();

            if (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN) {
                int pointerId = event.getPointerId(index);
                int control = controlAt(event.getX(index), event.getY(index));
                if (control != NONE) {
                    pointerControls.put(pointerId, control);
                    setPressed(control, true);
                }
                return true;
            }

            if (action == MotionEvent.ACTION_MOVE) {
                for (int i = 0; i < event.getPointerCount(); i++) {
                    int pointerId = event.getPointerId(i);
                    int oldControl = pointerControls.get(pointerId, NONE);
                    if (oldControl == NONE) continue;
                    int newControl = controlAt(event.getX(i), event.getY(i));
                    if (newControl != oldControl) {
                        setPressed(oldControl, false);
                        pointerControls.delete(pointerId);
                        if (newControl != NONE) {
                            pointerControls.put(pointerId, newControl);
                            setPressed(newControl, true);
                        }
                    }
                }
                return true;
            }

            if (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_POINTER_UP || action == MotionEvent.ACTION_CANCEL) {
                if (action == MotionEvent.ACTION_CANCEL) {
                    for (int i = 0; i < pointerControls.size(); i++)
                        setPressed(pointerControls.valueAt(i), false);
                    pointerControls.clear();
                } else {
                    releasePointer(event.getPointerId(index));
                }
                return true;
            }
            return true;
        }

        @Override
        protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            drawCircleButton(canvas, 54, 184, 43, (pressed[UP] || pressed[DOWN] || pressed[LEFT] || pressed[RIGHT]));
            drawCircleButton(canvas, 366, 181, 27, pressed[A]);
            drawCircleButton(canvas, 320, 202, 23, pressed[B]);
            drawRectButton(canvas, 185, 216, 56, 18, pressed[SELECT], "SELECT");
            drawRectButton(canvas, 123, 216, 56, 18, pressed[START], "START");
            drawRectButton(canvas, 76, 14, 40, 28, pressed[L], "L");
            drawRectButton(canvas, 310, 14, 40, 28, pressed[R], "R");
        }

        private void drawCircleButton(Canvas canvas, float cx, float cy, float radius, boolean down) {
            paint.setColor(0xFFFFFFFF);
            paint.setAlpha(down ? 110 : 55);
            canvas.drawCircle(sx(cx), sy(cy), sx(radius), paint);
            paint.setStyle(Paint.Style.STROKE);
            paint.setStrokeWidth(Math.max(2, sx(2)));
            paint.setAlpha(down ? 210 : 100);
            canvas.drawCircle(sx(cx), sy(cy), sx(radius), paint);
            paint.setStyle(Paint.Style.FILL);
        }

        private void drawRectButton(Canvas canvas, float cx, float cy, float w, float h, boolean down, String label) {
            paint.setColor(0xFFFFFFFF);
            paint.setAlpha(down ? 105 : 50);
            RectF r = new RectF(sx(cx), sy(cy), sx(cx + w), sy(cy + h));
            canvas.drawRoundRect(r, sy(7), sy(7), paint);
            paint.setColor(0xFF000000);
            paint.setAlpha(down ? 210 : 120);
            paint.setTextSize(sy(10));
            paint.setTextAlign(Paint.Align.CENTER);
            canvas.drawText(label, r.centerX(), r.centerY() - (paint.ascent() + paint.descent()) / 2, paint);
        }
    }
}
