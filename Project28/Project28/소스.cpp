#include <windows.h>
#include "resource.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BOARD_SIZE 40
#define SHAPE_COUNT 5

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HMENU hMenu;

BOOL isGridSolid = TRUE;
BOOL isBorderOn = TRUE;
BOOL isMoveOn = TRUE;
BOOL isInformOn = TRUE;

typedef struct {
    RECT rect;
    COLORREF color;
    BOOL selected;
    COLORREF borderColor;
} Shape;

Shape shapes[SHAPE_COUNT];
int selectedShapeIndex = -1;
BOOL isDrawing = FALSE;
POINT startPoint;
POINT endPoint;
POINT prevMousePos;

Shape CreateShape(RECT rect, COLORREF color) {
    Shape shape;
    shape.rect = rect;
    shape.color = color;
    shape.selected = FALSE;
    shape.borderColor = RGB(0, 0, 0);
    return shape;
}

COLORREF MixColors(COLORREF color1, COLORREF color2) {
    int r = (GetRValue(color1) + GetRValue(color2)) / 2;
    int g = (GetGValue(color1) + GetGValue(color2)) / 2;
    int b = (GetBValue(color1) + GetBValue(color2)) / 2;
    return RGB(r, g, b);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_VREDRAW | CS_HREDRAW, WindowProc, 0, 0, hInstance, LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), (HBRUSH)COLOR_WINDOW, NULL, L"WindowClass", LoadIcon(NULL, IDI_APPLICATION) };

    if (!RegisterClassEx(&wc)) return 0;

    // 메뉴 생성
    hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_GRID_40001, L"그리드: 실선");
    AppendMenu(hSubMenu, MF_STRING, ID_GRID_40002, L"그리드: 점선");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"그리드");
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40003, L"색상: 빨강");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40004, L"색상: 주황");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40005, L"색상: 노랑");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40006, L"색상: 초록");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40007, L"색상: 파랑");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40008, L"색상: 보라");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"색상");
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_BORDER_ON, L"테두리: 켜기");
    AppendMenu(hSubMenu, MF_STRING, ID_BORDER_OFF, L"테두리: 끄기");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"테두리");
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_MOVE_ON, L"이동: 켜기");
    AppendMenu(hSubMenu, MF_STRING, ID_MOVE_OFF, L"이동: 끄기");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"이동");
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_INFORM_ON, L"정보: 켜기");
    AppendMenu(hSubMenu, MF_STRING, ID_INFORM_OFF, L"정보: 끄기");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"정보");
    hSubMenu = CreatePopupMenu();

    HWND hwnd = CreateWindowEx(0, L"WindowClass", L"Drawing Board", WS_OVERLAPPEDWINDOW, 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, hMenu, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 그리드 그리기
        if (isGridSolid) {
            for (int x = 0; x < WINDOW_WIDTH; x += BOARD_SIZE) {
                for (int y = 0; y < WINDOW_HEIGHT; y += BOARD_SIZE) {
                    Rectangle(hdc, x, y, x + BOARD_SIZE, y + BOARD_SIZE);
                }
            }
        }
        else {
            // 점선 그리드 그리기
            HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0)); // 점선 펜 생성
            SelectObject(hdc, hPen);

            for (int x = 0; x < WINDOW_WIDTH; x += BOARD_SIZE) {
                for (int y = 0; y < WINDOW_HEIGHT; y += BOARD_SIZE) {
                    MoveToEx(hdc, x, y + BOARD_SIZE / 2, NULL); // 시작점 이동
                    LineTo(hdc, x + BOARD_SIZE, y + BOARD_SIZE / 2); // 점선 그리기
                }
            }

            for (int y = 0; y < WINDOW_HEIGHT; y += BOARD_SIZE) {
                for (int x = 0; x < WINDOW_WIDTH; x += BOARD_SIZE) {
                    MoveToEx(hdc, x + BOARD_SIZE / 2, y, NULL); // 시작점 이동
                    LineTo(hdc, x + BOARD_SIZE / 2, y + BOARD_SIZE); // 점선 그리기
                }
            }

            DeleteObject(hPen);
        }

        // 도형 그리기
        for (int i = 0; i < SHAPE_COUNT; ++i) {
            HBRUSH hBrush = CreateSolidBrush(shapes[i].color);
            SelectObject(hdc, hBrush);
            Rectangle(hdc, shapes[i].rect.left, shapes[i].rect.top, shapes[i].rect.right, shapes[i].rect.bottom);
            if (shapes[i].selected && isBorderOn) {
                HPEN hPen = CreatePen(PS_SOLID, 3, shapes[i].borderColor);
                SelectObject(hdc, hPen);
                SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Rectangle(hdc, shapes[i].rect.left - 2, shapes[i].rect.top - 2, shapes[i].rect.right + 2, shapes[i].rect.bottom + 2);
                DeleteObject(hPen);
            }
            DeleteObject(hBrush);
        }

        // 정보 표시
        if (isInformOn) {
            RECT infoRect = { 10, 10, 400, 100 }; // 정보 표시 영역
            WCHAR infoText[300];
            wsprintf(infoText, L"도형 개수: %d\n", SHAPE_COUNT);
            for (int i = 0; i < SHAPE_COUNT; ++i) {
                if (!IsRectEmpty(&shapes[i].rect)) {
                    WCHAR shapeInfo[100];
                    WCHAR colorInfo[50];
                    WCHAR sizeInfo[50];
                    WCHAR color[20];

                    // 도형의 색상 정보 추가
                    COLORREF shapeColor = shapes[i].color;
                    int red = GetRValue(shapeColor);
                    int green = GetGValue(shapeColor);
                    int blue = GetBValue(shapeColor);
                    wsprintf(color, L"RGB(%d,%d,%d)", red, green, blue);

                    // 각 도형의 가로세로 보드크기 정보 추가
                    wsprintf(sizeInfo, L"도형 %d: %dx%d, ", i + 1, (shapes[i].rect.right - shapes[i].rect.left) / BOARD_SIZE, (shapes[i].rect.bottom - shapes[i].rect.top) / BOARD_SIZE);

                    // 각 도형의 색상 정보 추가
                    wsprintf(colorInfo, L"색상: %s\n", color);

                    // 도형 정보 결합
                    wsprintf(shapeInfo, L"%s%s", sizeInfo, colorInfo);

                    // 전체 정보에 추가
                    wcscat_s(infoText, shapeInfo);
                }
            }
            DrawText(hdc, infoText, -1, &infoRect, DT_SINGLELINE | DT_NOCLIP);
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN: {
        isDrawing = TRUE;
        startPoint.x = LOWORD(lParam) / BOARD_SIZE * BOARD_SIZE; // 보드에 맞춤
        startPoint.y = HIWORD(lParam) / BOARD_SIZE * BOARD_SIZE;
        endPoint = startPoint;
        break;
    }
    case WM_MOUSEMOVE: {
        if (isDrawing) {
            HDC hdc = GetDC(hwnd);
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
            SelectObject(hdc, hBrush);
            SetROP2(hdc, R2_NOTXORPEN);
            Rectangle(hdc, startPoint.x, startPoint.y, endPoint.x, endPoint.y);
            endPoint.x = LOWORD(lParam) / BOARD_SIZE * BOARD_SIZE;
            endPoint.y = HIWORD(lParam) / BOARD_SIZE * BOARD_SIZE;
            Rectangle(hdc, startPoint.x, startPoint.y, endPoint.x, endPoint.y);
            DeleteObject(hBrush);
            ReleaseDC(hwnd, hdc);
        }
        else if (wParam & MK_RBUTTON && selectedShapeIndex != -1 && isMoveOn) {
            int deltaX = LOWORD(lParam) - prevMousePos.x;
            int deltaY = HIWORD(lParam) - prevMousePos.y;

            RECT* rect = &shapes[selectedShapeIndex].rect;
            rect->left += deltaX;
            rect->top += deltaY;
            rect->right += deltaX;
            rect->bottom += deltaY;

            prevMousePos.x = LOWORD(lParam);
            prevMousePos.y = HIWORD(lParam);

            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    }
    case WM_LBUTTONUP: {
        isDrawing = FALSE;
        int startX = startPoint.x;
        int startY = startPoint.y;
        int endX = endPoint.x + BOARD_SIZE;
        int endY = endPoint.y + BOARD_SIZE;

        HDC hdc = GetDC(hwnd);

        // 중첩된 도형 확인
        for (int i = 0; i < SHAPE_COUNT; ++i) {
            if (!IsRectEmpty(&shapes[i].rect) && PtInRect(&shapes[i].rect, startPoint) && PtInRect(&shapes[i].rect, endPoint)) {
                // 중첩된 영역 계산
                RECT overlapRect;
                overlapRect.left = max(startX, shapes[i].rect.left);
                overlapRect.top = max(startY, shapes[i].rect.top);
                overlapRect.right = min(endX, shapes[i].rect.right);
                overlapRect.bottom = min(endY, shapes[i].rect.bottom);

                // 색상 혼합
                COLORREF mixedColor = MixColors(shapes[i].color, RGB(0, 0, 0));
                HBRUSH hMixedBrush = CreateSolidBrush(mixedColor);
                SelectObject(hdc, hMixedBrush);
                Rectangle(hdc, overlapRect.left, overlapRect.top, overlapRect.right, overlapRect.bottom);
                DeleteObject(hMixedBrush);
                break;
            }
        }

        // 도형 추가
        for (int i = 0; i < SHAPE_COUNT; ++i) {
            if (IsRectEmpty(&shapes[i].rect)) {
                COLORREF color;
                // 빨강, 주황, 노랑, 초록, 파랑으로 순서대로 설정
                switch (i) {
                case 0:
                    color = RGB(255, 0, 0); // 빨강
                    break;
                case 1:
                    color = RGB(255, 165, 0); // 주황
                    break;
                case 2:
                    color = RGB(255, 255, 0); // 노랑
                    break;
                case 3:
                    color = RGB(0, 128, 0); // 초록
                    break;
                case 4:
                    color = RGB(0, 0, 255); // 파랑
                    break;
                default:
                    color = RGB(0, 0, 0); // 기본값은 검정색
                    break;
                }
                shapes[i] = CreateShape({ startX, startY, endX, endY }, color);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
        }

        ReleaseDC(hwnd, hdc);
        break;
    }
    case WM_RBUTTONDOWN: {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);

        // 도형 선택
        for (int i = 0; i < SHAPE_COUNT; ++i) {
            if (PtInRect(&shapes[i].rect, { xPos, yPos })) {
                selectedShapeIndex = i;
                prevMousePos.x = xPos;
                prevMousePos.y = yPos;
                break;
            }
        }
        break;
    }
    case WM_KEYDOWN: {
        if (selectedShapeIndex != -1) {
            RECT* rect = &shapes[selectedShapeIndex].rect;
            switch (wParam) {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
                // Reset border color of all shapes
                for (int i = 0; i < SHAPE_COUNT; ++i) {
                    shapes[i].selected = FALSE;
                    shapes[i].borderColor = RGB(0, 0, 0);
                }
                // Change border color of selected shape
                selectedShapeIndex = wParam - '1';
                if (selectedShapeIndex >= 0 && selectedShapeIndex < SHAPE_COUNT) {
                    shapes[selectedShapeIndex].selected = TRUE;
                    shapes[selectedShapeIndex].borderColor = RGB(255, 0, 0);
                }
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            case VK_LEFT:
                rect->left -= BOARD_SIZE;
                rect->right -= BOARD_SIZE;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            case VK_RIGHT:
                rect->left += BOARD_SIZE;
                rect->right += BOARD_SIZE;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            case VK_UP:
                rect->top -= BOARD_SIZE;
                rect->bottom -= BOARD_SIZE;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            case VK_DOWN:
                rect->top += BOARD_SIZE;
                rect->bottom += BOARD_SIZE;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
        }
        break;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_GRID_40001:
            isGridSolid = TRUE;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_GRID_40002:
            isGridSolid = FALSE;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40003:
            shapes[selectedShapeIndex].color = RGB(255, 0, 0); // 빨강
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40004:
            shapes[selectedShapeIndex].color = RGB(255, 165, 0); // 주황
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40005:
            shapes[selectedShapeIndex].color = RGB(255, 255, 0); // 노랑
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40006:
            shapes[selectedShapeIndex].color = RGB(0, 128, 0); // 초록
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40007:
            shapes[selectedShapeIndex].color = RGB(0, 0, 255); // 파랑
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40008:
            shapes[selectedShapeIndex].color = RGB(128, 0, 128); // 보라
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_BORDER_ON:
            isBorderOn = TRUE;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_BORDER_OFF:
            isBorderOn = FALSE;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_MOVE_ON:
            isMoveOn = TRUE;
            break;
        case ID_MOVE_OFF:
            isMoveOn = FALSE;
            break;
        case ID_INFORM_ON:
            isInformOn = TRUE;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_INFORM_OFF:
            isInformOn = FALSE;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        break;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}