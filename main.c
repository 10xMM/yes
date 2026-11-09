// IPTV Server - Switch NRO launcher
// Mirrors the Android app: stores your server address and opens its web UI.
#include <switch.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define CFG_DIR     "sdmc:/config/iptv-switch"
#define CFG_FILE    CFG_DIR "/server.txt"
#define DEFAULT_URL "http://192.168.1.103:8090"

static char g_url[256] = DEFAULT_URL;

static void load_cfg(void) {
    FILE *f = fopen(CFG_FILE, "r");
    if (!f) return;
    char buf[256] = {0};
    if (fgets(buf, sizeof(buf), f)) {
        buf[strcspn(buf, "\r\n")] = 0;
        if (strlen(buf) > 7) strcpy(g_url, buf);
    }
    fclose(f);
}

static void save_cfg(void) {
    mkdir("sdmc:/config", 0777);
    mkdir(CFG_DIR, 0777);
    FILE *f = fopen(CFG_FILE, "w");
    if (!f) return;
    fprintf(f, "%s\n", g_url);
    fclose(f);
}

static void edit_url(void) {
    SwkbdConfig kbd;
    char out[256] = {0};
    if (R_FAILED(swkbdCreate(&kbd, 0))) return;
    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetHeaderText(&kbd, "Server address");
    swkbdConfigSetGuideText(&kbd, "http://192.168.1.103:8090");
    swkbdConfigSetInitialText(&kbd, g_url);
    swkbdConfigSetStringLenMax(&kbd, 200);
    if (R_SUCCEEDED(swkbdShow(&kbd, out, sizeof(out))) && strlen(out) > 0) {
        if (strncmp(out, "http://", 7) && strncmp(out, "https://", 8)) {
            snprintf(g_url, sizeof(g_url), "http://%s", out);
        } else {
            strncpy(g_url, out, sizeof(g_url) - 1);
        }
        save_cfg();
    }
    swkbdClose(&kbd);
}

static void open_web(void) {
    WebCommonConfig web;
    WebCommonReply reply;
    Result rc = webPageCreate(&web, g_url);
    if (R_SUCCEEDED(rc)) {
        webConfigSetWhitelist(&web, "^http.*");
        rc = webConfigShow(&web, &reply);
    }
    if (R_FAILED(rc)) {
        printf("\nCould not open browser (0x%x).\n", rc);
        printf("Launch hbmenu from the Album (applet mode),\nnot by overriding a game.\n");
    }
}

static void draw(void) {
    consoleClear();
    printf("\x1b[1;1H=== IPTV Server ===\n\n");
    printf("Server: %s\n\n", g_url);
    printf("  A : Open server\n");
    printf("  X : Change server address\n");
    printf("  + : Exit\n");
    consoleUpdate(NULL);
}

int main(void) {
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    load_cfg();
    draw();

    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 down = padGetButtonsDown(&pad);
        if (down & HidNpadButton_Plus) break;
        if (down & HidNpadButton_X) { edit_url(); draw(); }
        if (down & HidNpadButton_A) { open_web(); draw(); }
        consoleUpdate(NULL);
    }
    consoleExit(NULL);
    return 0;
}
