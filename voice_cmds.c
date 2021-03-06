#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "utils.h"
#include "vocli.h"
#include "voice_edit.h"
#include "voice_cmds.h"

// Command table
const VoiceCmd VoiceCommands[] = {
    {"print",   &ve_cmd_print},
    {"edit",    &ve_cmd_edit},
    {"name",    &ve_cmd_name},
    {"save",    &ve_cmd_save},
    {"help",    &ve_cmd_help},
    {"quit",    &ve_cmd_quit}
};

// Command count
const int num_voice_cmds = sizeof(VoiceCommands) / sizeof(VoiceCommands[0]);

/* Phoneme print command */
int ve_cmd_print(char *input, VoiceDef *voice)
{
    // Help text
    if (input == NULL)
        return error(FAIL, "print <Name/ID>\t--\tPrint the phoneme specified by <Name/ID>");

    strtok(input, " ");

    // Get phoneme ID in string format
    char *IDstr = strtok(NULL, " ");
    if (IDstr == NULL)
        return error(FAIL, "No phoneme specified\n");

    strchomp(IDstr);

    // Get ID from input string
    uint8_t phonID = 255;
    if (isalpha(IDstr[0]))
        phonID = get_phoneme_ID(IDstr);
    else
        phonID = (uint8_t)strtol(IDstr, NULL, 10);

    // Check ID
    if (phonID >= NUM_PHONEMES)
        return error(FAIL, "Invalid phoneme ID '%s'\n", IDstr);

    // Output phoneme data
    print_phoneme(voice->Phoneme[phonID]);

    return SUCCESS;
}

/* Phoneme edit command */
int ve_cmd_edit(char *input, VoiceDef *voice)
{
    // Help text
    if (input == NULL)
        return error(FAIL, "edit <Name/ID>\t--\tEdit the phoneme specified by <Name/ID>");

    strtok(input, " ");
    char *IDstr = strtok(NULL, " ");
    if (IDstr == NULL)
        return error(FAIL, "No phoneme specified\n");

    strchomp(IDstr);

    // Determine if IDstr is ID or name
    uint8_t phonID;
    if (isalpha(IDstr[0]))
        phonID = get_phoneme_ID(IDstr);
    else
        phonID = (uint8_t)strtol(IDstr, NULL, 10);

    // Check ID
    if (phonID >= NUM_PHONEMES)
        return error(FAIL, "Invalid phoneme ID '%s'\n", IDstr);

    // Pick a formant to edit
    uint8_t fmtID = 255;
    do {
        printf("Which formant would you like to edit? [1 - %d?, 0 = all]: ", NUM_FORMANTS);
        fgets(IDstr, 12, stdin);
        if (IDstr != NULL)
            fmtID = IDstr[0] - '0';
    } while (fmtID > NUM_FORMANTS);    

    // Edit selected formant(s)
    for (int i = 0; i < 3; i++)
    {
        if (fmtID == 0 || fmtID == i + 1)
        {
            // Shorthand formant reference
            FmtDef *formant = &voice->Phoneme[phonID].Formant[i];

            // Buffers
            char fqc[12], bw[12], dBoffs[12];

            // Read frequency
            formant->fqc = 0;
            do {
                printf("Formant #%d frequency: ", i + 1);
                fgets(fqc, 12, stdin);
                formant->fqc = labs(strtol(fqc, NULL, 10));
            } while (formant->fqc == 0);

            // Read bandwidth
            formant->bw = 0;
            do {
                printf("Formant #%d bandwidth: ", i + 1);
                fgets(bw, 12, stdin);
                formant->bw = labs(strtol(bw, NULL, 10));
            } while (formant->bw == 0);

            // Read dB offset
            do {
                formant->dBoffs = 0;
                printf("Formant #%d dB offset: ", i + 1);
                fgets(dBoffs, 12, stdin);
                formant->dBoffs = strtol(dBoffs, NULL, 10);
            } while (i > 0 && formant->dBoffs == 0);
        }
    }

    // Print phoneme after editing
    print_phoneme(voice->Phoneme[phonID]);

    return SUCCESS;
}

/* Voice rename command */
int ve_cmd_name(char *input, VoiceDef *voice)
{
    // Help text
    if (input == NULL)
        return error(FAIL, "name <new>\t--\tRename the current voice to the value specified by <new>");

    // Extract name
    // TODO: Use a different function to manually extract _ALL_ text after the first space, including spaces
    strtok(input, " ");
    char *name = strtok(NULL, " ");

    // Check for a name
    if (name == NULL || strchomp(name)[0] == '\0')
        return error(FAIL, "Please specify a name\n");

    // Verify name
    if (strlen(name) >= SPEAKER_MAX)
        return error(FAIL, "The name you have specified is too long\n");

    printf("Changing name from '%s' to '%s'\n", voice->name, name);  
    strncpy(voice->name, name, strlen(name) + 1);

    return SUCCESS;
}

/* Voice save command */
int ve_cmd_save(char *input, VoiceDef *voice)
{
    // Help text
    if (input == NULL)
        return error(FAIL, "save [file]\t--\tSave the current voice to the specified or last used filename");

    // Get specified filename
    strtok(input, " ");
    char *newfile = strtok(NULL, " ");

    // Verify a filename is present
    if (newfile == NULL)
    {
        if (voice->filename[0] == '\0')
            return error(FAIL, "Please specify a filename\n");
    }
    else
    {
        strncpy(voice->filename, strchomp(newfile), PATH_MAX);
    }

    printf("filename: %s\n", voice->filename);

    // Save voice to file  
    write_voice(voice->filename, voice);

    return SUCCESS;
}

/* Help command */
int ve_cmd_help(char *input, VoiceDef *voice)
{
    // Meta-help text
    if (input == NULL)
        return error(FAIL, "help\t\t--\tDisplay this help text");

    (void)(voice);

    // Call every command with NULL input
    for (int i = 0; i < num_voice_cmds; i++)
    {
        VoiceCommands[i].callback(NULL, NULL);
    }

    return SUCCESS;
}

/* Quit command */
int ve_cmd_quit(char *input, VoiceDef *voice)
{
    // Help text
    if (input == NULL)
        return error(FAIL, "quit\t\t--\tQuit the voice editor and return to Vocli");

    // Save voice before exiting
    if (voice->filename[0] != '\0')
    {
        printf("Auto-saving...\n");
        write_voice(voice->filename, voice);
    }

    //exit(SUCCESS);
}

