package com.example.opengles_learning;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;

public class MainActivity extends AppCompatActivity {

    private static String LOGTAG = "SimpleTriangle";

    protected TutorialView graphicsView;

    @Override protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Log.i(LOGTAG, "Creating New Tutorial View");
        graphicsView = new TutorialView(getApplication());
        setContentView(graphicsView);
    }

    @Override protected void onPause()
    {
        super.onPause();
        graphicsView.onPause();
    }

    @Override protected void onResume()
    {
        super.onResume();
        graphicsView.onResume();
    }
}