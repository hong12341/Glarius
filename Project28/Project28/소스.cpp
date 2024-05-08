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

    // �޴� ����
    hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_GRID_40001, L"�׸���: �Ǽ�");
    AppendMenu(hSubMenu, MF_STRING, ID_GRID_40002, L"�׸���: ����");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"�׸���");
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40003, L"����: ����");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40004, L"����: ��Ȳ");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40005, L"����: ���");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40006, L"����: �ʷ�");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40007, L"����: �Ķ�");
    AppendMenu(hSubMenu, MF_STRING, ID_COLOR_40008, L"����: ����");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"����");
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_BORDER_ON, L"�׵θ�: �ѱ�");
    AppendMenu(hSubMenu, MF_STRING, ID_BORDER_OFF, L"�׵θ�: ����");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"�׵θ�");
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_MOVE_ON, L"�̵�: �ѱ�");
    AppendMenu(hSubMenu, MF_STRING, ID_MOVE_OFF, L"�̵�: ����");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"�̵�");
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_INFORM_ON, L"����: �ѱ�");
    AppendMenu(hSubMenu, MF_STRING, ID_INFORM_OFF, L"����: ����");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"����");
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

        // �׸��� �׸���
        if (isGridSolid) {
            for (int x = 0; x < WINDOW_WIDTH; x += BOARD_SIZE) {
                for (int y = 0; y < WINDOW_HEIGHT; y += BOARD_SIZE) {
                    Rectangle(hdc, x, y, x + BOARD_SIZE, y + BOARD_SIZE);
                }
            }
        }
        else {
            // ���� �׸��� �׸���
            HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0)); // ���� �� ����
            SelectObject(hdc, hPen);

            for (int x = 0; x < WINDOW_WIDTH; x += BOARD_SIZE) {
                for (int y = 0; y < WINDOW_HEIGHT; y += BOARD_SIZE) {
                    MoveToEx(hdc, x, y + BOARD_SIZE / 2, NULL); // ������ �̵�
                    LineTo(hdc, x + BOARD_SIZE, y + BOARD_SIZE / 2); // ���� �׸���
                }
            }

            for (int y = 0; y < WINDOW_HEIGHT; y += BOARD_SIZE) {
                for (int x = 0; x < WINDOW_WIDTH; x += BOARD_SIZE) {
                    MoveToEx(hdc, x + BOARD_SIZE / 2, y, NULL); // ������ �̵�
                    LineTo(hdc, x + BOARD_SIZE / 2, y + BOARD_SIZE); // ���� �׸���
                }
            }

            DeleteObject(hPen);
        }

        // ���� �׸���
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

        // ���� ǥ��
        if (isInformOn) {
            RECT infoRect = { 10, 10, 400, 100 }; // ���� ǥ�� ����
            WCHAR infoText[300];
            wsprintf(infoText, L"���� ����: %d\n", SHAPE_COUNT);
            for (int i = 0; i < SHAPE_COUNT; ++i) {
                if (!IsRectEmpty(&shapes[i].rect)) {
                    WCHAR shapeInfo[100];
                    WCHAR colorInfo[50];
                    WCHAR sizeInfo[50];
                    WCHAR color[20];

                    // ������ ���� ���� �߰�
                    COLORREF shapeColor = shapes[i].color;
                    int red = GetRValue(shapeColor);
                    int green = GetGValue(shapeColor);
                    int blue = GetBValue(shapeColor);
                    wsprintf(color, L"RGB(%d,%d,%d)", red, green, blue);

                    // �� ������ ���μ��� ����ũ�� ���� �߰�
                    wsprintf(sizeInfo, L"���� %d: %dx%d, ", i + 1, (shapes[i].rect.right - shapes[i].rect.left) / BOARD_SIZE, (shapes[i].rect.bottom - shapes[i].rect.top) / BOARD_SIZE);

                    // �� ������ ���� ���� �߰�
                    wsprintf(colorInfo, L"����: %s\n", color);

                    // ���� ���� ����
                    wsprintf(shapeInfo, L"%s%s", sizeInfo, colorInfo);

                    // ��ü ������ �߰�
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
        startPoint.x = LOWORD(lParam) / BOARD_SIZE * BOARD_SIZE; // ���忡 ����
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

        // ��ø�� ���� Ȯ��
        for (int i = 0; i < SHAPE_COUNT; ++i) {
            if (!IsRectEmpty(&shapes[i].rect) && PtInRect(&shapes[i].rect, startPoint) && PtInRect(&shapes[i].rect, endPoint)) {
                // ��ø�� ���� ���
                RECT overlapRect;
                overlapRect.left = max(startX, shapes[i].rect.left);
                overlapRect.top = max(startY, shapes[i].rect.top);
                overlapRect.right = min(endX, shapes[i].rect.right);
                overlapRect.bottom = min(endY, shapes[i].rect.bottom);

                // ���� ȥ��
                COLORREF mixedColor = MixColors(shapes[i].color, RGB(0, 0, 0));
                HBRUSH hMixedBrush = CreateSolidBrush(mixedColor);
                SelectObject(hdc, hMixedBrush);
                Rectangle(hdc, overlapRect.left, overlapRect.top, overlapRect.right, overlapRect.bottom);
                DeleteObject(hMixedBrush);
                break;
            }
        }

        // ���� �߰�
        for (int i = 0; i < SHAPE_COUNT; ++i) {
            if (IsRectEmpty(&shapes[i].rect)) {
                COLORREF color;
                // ����, ��Ȳ, ���, �ʷ�, �Ķ����� ������� ����
                switch (i) {
                case 0:
                    color = RGB(255, 0, 0); // ����
                    break;
                case 1:
                    color = RGB(255, 165, 0); // ��Ȳ
                    break;
                case 2:
                    color = RGB(255, 255, 0); // ���
                    break;
                case 3:
                    color = RGB(0, 128, 0); // �ʷ�
                    break;
                case 4:
                    color = RGB(0, 0, 255); // �Ķ�
                    break;
                default:
                    color = RGB(0, 0, 0); // �⺻���� ������
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

        // ���� ����
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
            shapes[selectedShapeIndex].color = RGB(255, 0, 0); // ����
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40004:
            shapes[selectedShapeIndex].color = RGB(255, 165, 0); // ��Ȳ
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40005:
            shapes[selectedShapeIndex].color = RGB(255, 255, 0); // ���
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40006:
            shapes[selectedShapeIndex].color = RGB(0, 128, 0); // �ʷ�
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40007:
            shapes[selectedShapeIndex].color = RGB(0, 0, 255); // �Ķ�
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_COLOR_40008:
            shapes[selectedShapeIndex].color = RGB(128, 0, 128); // ����
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