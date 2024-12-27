#include <windows.h>
#include <iostream>

// Global variables for the widget windows
HWND hWidget1, hWidget2, hold;
bool isDragging = false;  // Flag to track if we are dragging
POINT dragStartPoint;     // The starting point of the drag (initial mouse position)
POINT widgetStartPos;     // The starting position of the widget
int dy = 0;
int dx = 0;
int newY = 0;
int newX = 0;
const int THRESHOLD = 10; // Adjust this value to change sensitivity
// Determine the primary direction of movement
enum Direction {
    DirectionNone,
    DirectionUp,
    DirectionDown,
    DirectionLeft,
    DirectionRight,
    DirectionDL,
    DirectionDR,
    DirectionRU,
    DirectionLU
} primaryDirection;
LPARAM lP;
WPARAM wP;

LRESULT CALLBACK WidgetProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void CreateWidgets() {
    // Register the window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WidgetProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"DesktopWidget";

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register widget window class. Error: " << GetLastError() << std::endl;
        return;
    }

    // Create the first widget
    hWidget1 = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED,
        L"DesktopWidget",
        NULL,
        WS_POPUP,
        100, 100, 200, 200,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    if (!hWidget1) {
        std::cerr << "Failed to create first widget window. Error: " << GetLastError() << std::endl;
        return;
    }

    SetLayeredWindowAttributes(hWidget1, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(hWidget1, SW_SHOW);
    UpdateWindow(hWidget1);

    // Create the second widget
    hWidget2 = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED,
        L"DesktopWidget",
        NULL,
        WS_POPUP,
        350, 100, 200, 200,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    if (!hWidget2) {
        std::cerr << "Failed to create second widget window. Error: " << GetLastError() << std::endl;
        return;
    }

    SetLayeredWindowAttributes(hWidget2, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(hWidget2, SW_SHOW);
    UpdateWindow(hWidget2);

    // Message loop for widgets
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Window procedure for widgets
LRESULT CALLBACK WidgetProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (hwnd == hWidget1) {
            SetTextColor(hdc, RGB(255, 77, 88));
            SetBkMode(hdc, TRANSPARENT);
            Rectangle(hdc, 7, 7, 500, 500);
            TextOut(hdc, 10, 10, L"YOUR GAY!!", 14);
        }
        else if (hwnd == hWidget2) {
            SetTextColor(hdc, RGB(0, 77, 0));
            SetBkMode(hdc, OPAQUE);
            Rectangle(hdc, 10, 10, 150, 150);
            TextOut(hdc, 20, 20, L"IPA'S ON WINDOWS", 18);
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN: {
        // Start dragging the widget
        isDragging = true;
        dragStartPoint.x = LOWORD(lParam);
        dragStartPoint.y = HIWORD(lParam);
        // Get the widget's current position relative to the client area
        RECT rect;
        GetWindowRect(hwnd, &rect);
        widgetStartPos.x = rect.left;
        widgetStartPos.y = rect.top;
        SetCapture(hwnd);  // Capture mouse input
        hold = hwnd;
        std::cout << hwnd << std::endl;
        break;
    }

    case WM_LBUTTONUP: {
        // Stop dragging the widget
        isDragging = false;
        ReleaseCapture();  // Release mouse capture
        break;
    }

    case WM_MOUSEMOVE: {
        /*if (isDragging) {
            if (dragStartPoint.x > dragStartPoint.x + 1) {
                dx = LOWORD(lParam) - dragStartPoint.x;
            }
            if (dragStartPoint.x < dragStartPoint.x - 1) {
                dx = LOWORD(lParam) + dragStartPoint.x;
            }
            if (dragStartPoint.y > dragStartPoint.y + 1) {
                dy = LOWORD(lParam) - dragStartPoint.y;
            }
            if (dragStartPoint.y < dragStartPoint.y - 1) {
                dy = LOWORD(lParam) + dragStartPoint.y;
            }

            // Check if the movement is towards the right or left for X-axis
            if (dx > dx + 1) {
                newX = widgetStartPos.x - dx;
            }
            else if (dx < dx - 1) {
                newX = widgetStartPos.x + dx;
            }
            if (dy > dy + 1) {
                newY = widgetStartPos.y - dy;
            }
            else if (dy < dx - 1) {
                newY = widgetStartPos.y + dy;
            }

            if (newY > 1800) {
                newY = newY - 1800;
            }

            if (newX > 1800) {
                newX = newX - 1800;
            }

            // Ensure the window doesn't move outside the screen bounds
            RECT screenRect;
            GetClientRect(GetDesktopWindow(), &screenRect);
            int screenWidth = screenRect.right;
            int screenHeight = screenRect.bottom;

            // Restrict the window to stay within the screen bounds
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX + 200 > screenWidth) newX = screenWidth - 200;
            if (newY + 200 > screenHeight) newY = screenHeight - 200;

            std::cout << "WTF pos: X: " << newX << " Y: " << newY << std::endl;

            // Move the widget to the new position
            SetWindowPos(hwnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }*/
        if (isDragging) {
            // Calculate the change in position
            int dx = LOWORD(lParam) - dragStartPoint.x;
            int dy = HIWORD(lParam) - dragStartPoint.y;

            // Determine the direction based on dx and dy
            if (abs(dx) < THRESHOLD && abs(dy) < THRESHOLD) {
                primaryDirection = DirectionNone;
            }
            else if (abs(dx) > abs(dy) * 1.5) {
                primaryDirection = (dx > 0) ? DirectionRight : DirectionLeft;
            }
            else if (abs(dy) > abs(dx) * 1.5) {
                primaryDirection = (dy > 0) ? DirectionDown : DirectionUp;
            }
            else {
                if (dx >= 0 && dy >= 0) primaryDirection = DirectionDR;
                else if (dx >= 0 && dy < 0) primaryDirection = DirectionRU;
                else if (dx < 0 && dy >= 0) primaryDirection = DirectionDL;
                else primaryDirection = DirectionLU;
            }

            // Calculate new position based on primary direction
            int newX = widgetStartPos.x;
            int newY = widgetStartPos.y;

            /*switch (primaryDirection) {
            case DirectionUp:
                newY = widgetStartPos.y - dy;
                break;
            case DirectionDown:
                newY = widgetStartPos.y + dy;
                break;
            case DirectionDL:
                newY = widgetStartPos.y + dy;
                newX = widgetStartPos.x - dx;
                break;
            case DirectionDR:
                newY = widgetStartPos.y + dy;
                newX = widgetStartPos.x + dx;
                break;
            case DirectionLeft:
                newX = widgetStartPos.x - dx;
                break;
            case DirectionLU:
                newX = widgetStartPos.x - dx;
                newY = widgetStartPos.y - dy;
                break;
            case DirectionRight:
                newX = widgetStartPos.x + dx;
                break;
            case DirectionRU:
                newX = widgetStartPos.x + dx;
                newY = widgetStartPos.y - dy;
                break;
            case DirectionNone:
                // No significant movement, keep the original position
                break;
            }*/

            switch (primaryDirection) {
            case DirectionUp:
                newX = widgetStartPos.x; // Keep X unchanged
                newY = dragStartPoint.y + dy; // Align Y to the mouse position
                break;
            case DirectionDown:
                newX = widgetStartPos.x; // Keep X unchanged
                newY = dragStartPoint.y + dy; // Align Y to the mouse position
                break;
            case DirectionLeft:
                newX = dragStartPoint.x + dx; // Align X to the mouse position
                newY = widgetStartPos.y; // Keep Y unchanged
                break;
            case DirectionRight:
                newX = dragStartPoint.x + dx; // Align X to the mouse position
                newY = widgetStartPos.y; // Keep Y unchanged
                break;
            case DirectionDL: // Diagonal Down-Left
                newX = dragStartPoint.x + dx; // Align X to the mouse position
                newY = dragStartPoint.y + dy; // Align Y to the mouse position
                break;
            case DirectionDR: // Diagonal Down-Right
                newX = dragStartPoint.x + dx; // Align X to the mouse position
                newY = dragStartPoint.y + dy; // Align Y to the mouse position
                break;
            case DirectionLU: // Diagonal Up-Left
                newX = dragStartPoint.x + dx; // Align X to the mouse position
                newY = dragStartPoint.y + dy; // Align Y to the mouse position
                break;
            case DirectionRU: // Diagonal Up-Right
                newX = dragStartPoint.x + dx; // Align X to the mouse position
                newY = dragStartPoint.y + dy; // Align Y to the mouse position
                break;
            case DirectionNone:
                // No significant movement, keep original position
                newX = widgetStartPos.x;
                newY = widgetStartPos.y;
                break;
            }

            if (newY > 1800) {
                newY = newY - 1800;
            }

            if (newX > 1800) {
                newX = newX - 1800;
            }

            // Ensure the window doesn't move outside the screen bounds
            RECT screenRect;
            GetClientRect(GetDesktopWindow(), &screenRect);
            int screenWidth = screenRect.right;
            int screenHeight = screenRect.bottom;

            // Restrict the window to stay within the screen bounds
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX + 200 > screenWidth) newX = screenWidth - 200;
            if (newY + 200 > screenHeight) newY = screenHeight - 200;

            std::cout << "New position: X: " << newX << " Y: " << newY << std::endl;

            // Move the widget to the new position
            SetWindowPos(hwnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        break;
    }

    case WM_CLOSE:
    case WM_DESTROY:
        // Prevent widgets from being destroyed when closed
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void DBG() {
    DWORD original_protection;
    VirtualProtect(&FreeConsole, sizeof(uint8_t), PAGE_EXECUTE_READWRITE, &original_protection);
    *(uint8_t*)(&FreeConsole) = 0xC3;
    VirtualProtect(&FreeConsole, sizeof(uint8_t), original_protection, NULL);
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CONIN$", "r", stdin);
    freopen_s(&stream, "CONOUT$", "w", stdout);
}

// Entry point for DLL injection
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DBG();
        std::cout << "LUE Loaded" << std::endl;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(CreateWidgets), NULL, 0, NULL);
        //CreateWidgets();  // Create widgets once the DLL is loaded
    }
    return TRUE;
}
