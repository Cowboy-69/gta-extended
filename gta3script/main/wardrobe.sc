// Vice Extended (ViceEx)
// *****************************************************************************************
// ************************************Player's wardrobe************************************
// *****************************************************************************************

MISSION_START

SCRIPT_NAME wardrob

SET_DEATHARREST_STATE OFF

mission_wardrobe_start:
	START_NEW_SCRIPT wardrobe_start

	MISSION_END

wardrobe_start:

SCRIPT_NAME WRDRB

{

LVAR_FLOAT wardrobePosX, wardrobePosY, wardrobePosZ, wardrobeHeading
LVAR_INT currentPickupInUse
LVAR_INT storedVariation, currentVariation
LVAR_INT storedWardrobeClothingNumber, currentWardrobeClothingNumber

wardrobePosX = 0.0 // 139.0
wardrobePosY = 0.0 // -822.0
wardrobePosZ = 0.0 // 10.0
wardrobeHeading = 0.0
currentPickupInUse = 0
storedVariation = 0
currentVariation = 0
storedWardrobeClothingNumber = 0
currentWardrobeClothingNumber = 1

// debug
//CREATE_CLOTHES_PICKUP 226.4 -1265.6 20.1 1 clothes_pickup1
//CREATE_CLOTHES_PICKUP -384.5 -591.9 25.3 1 mansion_clothes
//CREATE_CLOTHES_PICKUP -820.2 1364.1 66.4 1 safehouse_clothes2
/*CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup4
CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup8
CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup2
CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup11
CREATE_CLOTHES_PICKUP 106.5 253.0 21.7 4 clothes_pickup7*/

// debug
//hotel_clothes_created = 1 // Street - OUTFT1
//mansion_clothes_created = 1 // Street - OUTFT1
//safehouse_created2 = 1 // Street - OUTFT1
/*clothes2_created = 1 // Soiree - OUTFT2
clothes4_created = 1 // Country Club - OUTFT4
clothes7_created = 1 // Casual - OUTFT8
clothes8_created = 1 // Mr Vercetti - OUTFT9
clothes9_created = 1 // Tracksuit - OUTFT10
clothes3_created = 1 // Coveralls - OUTFT3
clothes5_created = 1 // Havana - OUTFT5
clothes6_created = 1 // Cop - OUTFT6
clothes13_created = 1 // Bank Job - OUTFT7
created_final_shirt = 1 // Frankie - OUTFT12
*/

wardrobe_loop:
    WAIT 0 

    if hotel_clothes_created = 1
        if HAS_PICKUP_BEEN_COLLECTED clothes_pickup1
            REMOVE_PICKUP clothes_pickup1

            currentPickupInUse = 1

            wardrobePosX = 228.0
            wardrobePosY = -1266.0
            wardrobePosZ = 20.1
            wardrobeHeading = -100.0

            GOTO wardrobe_pickup_collected
        endif
    endif

    if mansion_clothes_created = 1
        if HAS_PICKUP_BEEN_COLLECTED mansion_clothes
            REMOVE_PICKUP mansion_clothes

            currentPickupInUse = 2

            wardrobePosX = -381.5
            wardrobePosY = -591.9
            wardrobePosZ = 25.3
            wardrobeHeading = -90.0

            GOTO wardrobe_pickup_collected
        endif
    endif

    if safehouse_created2 = 1
        if HAS_PICKUP_BEEN_COLLECTED safehouse_clothes2
            REMOVE_PICKUP safehouse_clothes2

            currentPickupInUse = 3

            wardrobePosX = -820.2
            wardrobePosY = 1362.1
            wardrobePosZ = 66.4
            wardrobeHeading = 180.0

            GOTO wardrobe_pickup_collected
        endif
    endif

    GOTO wardrobe_loop

wardrobe_pickup_collected:
    SET_PLAYER_CONTROL 0 FALSE

    DO_FADE 1000 FADE_OUT
    WAIT 1000

    CLEAR_ALL_CHAR_ANIMS scplayer
    LVAR_FLOAT playerPosY
    playerPosY = wardrobePosZ - 1.5
    SET_CHAR_COORDINATES scplayer, wardrobePosX, wardrobePosY, playerPosY
    SET_CHAR_HEADING scplayer, wardrobeHeading

    LVAR_FLOAT camPosX, camPosY, camPosZ
    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS (scplayer, 0.0, 2.5, 1.0, camPosX, camPosY, camPosZ)
    SET_FIXED_CAMERA_POSITION (camPosX, camPosY, camPosZ, 0.0, 0.0, 0.0)
    POINT_CAMERA_AT_CHAR (scplayer 15 2)

    GET_CHAR_CLOTHING_VARIATION scplayer storedVariation
    currentVariation = storedVariation

    // Street
    if IS_PLAYER_WEARING player1 player
        if storedVariation = 0
            storedWardrobeClothingNumber = 1
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 13
            else
                storedWardrobeClothingNumber = 20
            endif
        endif
    endif

    // Soiree
    if IS_PLAYER_WEARING player1 PLAYER2
        if storedVariation = 0
            storedWardrobeClothingNumber = 2
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 14
            else
                storedWardrobeClothingNumber = 21
            endif
        endif
    endif

    // Country Club
    if IS_PLAYER_WEARING player1 PLAYER4
        if storedVariation = 0
            storedWardrobeClothingNumber = 3
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 15
            else
                storedWardrobeClothingNumber = 22
            endif
        endif
    endif

    // Casual
    if IS_PLAYER_WEARING player1 PLAYER8
        if storedVariation = 0
            storedWardrobeClothingNumber = 4
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 16
            else
                storedWardrobeClothingNumber = 23
            endif
        endif
    endif

    // Mr Vercetti
    if IS_PLAYER_WEARING player1 PLAYER9
        if storedVariation = 0
            storedWardrobeClothingNumber = 5
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 17
            else
                storedWardrobeClothingNumber = 24
            endif
        endif
    endif

    // Black tracksuit
    if IS_PLAYER_WEARING player1 PLAY10
        if storedVariation = 0
            storedWardrobeClothingNumber = 6
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 18
            else
                storedWardrobeClothingNumber = 25
            endif
        endif
    endif

    // Red tracksuit
    if IS_PLAYER_WEARING player1 PLAY11
        if storedVariation = 0
            storedWardrobeClothingNumber = 7
        else
            if storedVariation = 1
                storedWardrobeClothingNumber = 19
            else
                storedWardrobeClothingNumber = 26
            endif
        endif
    endif

    // Coveralls
    if IS_PLAYER_WEARING player1 PLAYER3
        storedWardrobeClothingNumber = 8
    endif

    // Havana
    if IS_PLAYER_WEARING player1 PLAYER5
        storedWardrobeClothingNumber = 9
    endif

    // Cop
    if IS_PLAYER_WEARING player1 PLAYER6
        storedWardrobeClothingNumber = 10
    endif

    // Bank Job
    if IS_PLAYER_WEARING player1 PLAYER7
        storedWardrobeClothingNumber = 11
    endif

    // Frankie
    if IS_PLAYER_WEARING player1 PLAY12
        storedWardrobeClothingNumber = 12
    endif

    currentWardrobeClothingNumber = storedWardrobeClothingNumber

    DO_FADE 1000 FADE_IN
    WAIT 1000

wardrobe_player_loop:
    WAIT 0

    GOSUB wardrobe_display_info

    // Switching clothing variations
    LVAR_INT leftStick_X, unused
    GET_POSITION_OF_ANALOGUE_STICKS 0, leftStick_X, unused, unused, unused
    if (leftStick_X < -64)
        GOSUB wardrobe_before_change_clothes_prev
        GOSUB wardrobe_change_clothes
    else
        if (leftStick_X > 64)
            GOSUB wardrobe_before_change_clothes_next
            GOSUB wardrobe_change_clothes
        endif
    endif

    // Put on clothes
    if (IS_BUTTON_PRESSED PAD1 CROSS)
        GOSUB wardrobe_put_on_clothes
    endif

    if not IS_BUTTON_PRESSED PAD1 TRIANGLE)
        GOTO wardrobe_player_loop
    endif

    // Leave wardrobe
    while (IS_BUTTON_PRESSED PAD1 TRIANGLE)
        WAIT 0
    endwhile

    GOTO leave_wardrobe

wardrobe_display_info:
    PRINT_HELP_FOREVER CLOTH_4

    USE_TEXT_COMMANDS TRUE

    // Displaying clothing name

    // Street
    if (currentWardrobeClothingNumber = 1)
    or (currentWardrobeClothingNumber = 13)
    or (currentWardrobeClothingNumber = 20)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT1
    endif

    // Soiree
    if (currentWardrobeClothingNumber = 2)
    or (currentWardrobeClothingNumber = 14)
    or (currentWardrobeClothingNumber = 21)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT2
    endif

    // Country Club
    if (currentWardrobeClothingNumber = 3)
    or (currentWardrobeClothingNumber = 15)
    or (currentWardrobeClothingNumber = 22)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT4
    endif

    // Casual
    if (currentWardrobeClothingNumber = 4)
    or (currentWardrobeClothingNumber = 16)
    or (currentWardrobeClothingNumber = 23)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT8
    endif

    // Mr Vercetti
    if (currentWardrobeClothingNumber = 5)
    or (currentWardrobeClothingNumber = 17)
    or (currentWardrobeClothingNumber = 24)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT9
    endif

    // Black tracksuit
    if (currentWardrobeClothingNumber = 6)
    or (currentWardrobeClothingNumber = 18)
    or (currentWardrobeClothingNumber = 25)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT10
    endif

    // Red tracksuit
    if (currentWardrobeClothingNumber = 7)
    or (currentWardrobeClothingNumber = 19)
    or (currentWardrobeClothingNumber = 26)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT10
    endif

    // Coveralls
    if (currentWardrobeClothingNumber = 8)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT3
    endif

    // Havana
    if (currentWardrobeClothingNumber = 9)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT5
    endif

    // Cop
    if (currentWardrobeClothingNumber = 10)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT6
    endif

    // Bank Job
    if (currentWardrobeClothingNumber = 11)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT7
    endif

    // Frankie
    if (currentWardrobeClothingNumber = 12)
        SET_TEXT_CENTRE TRUE
        SET_TEXT_COLOUR 0 207 133 255
        SET_TEXT_SCALE 1.2 2.5
        DISPLAY_TEXT 320.0 45.0 OUTFT12
    endif

    if (currentWardrobeClothingNumber = storedWardrobeClothingNumber)
        DISPLAY_TEXT 320.0 425.0 CLOTH_5
    endif

    USE_TEXT_COMMANDS FALSE

    RETURN

wardrobe_before_change_clothes_prev:
    if (currentWardrobeClothingNumber = 1)
        currentWardrobeClothingNumber = 26

        if (redTracksuitVariationState = 2)
        or (redTracksuitVariationState = 3)
            currentWardrobeClothingNumber = 26
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 26)
        if (blackTracksuitVariationState = 2)
        or (blackTracksuitVariationState = 3)
            currentWardrobeClothingNumber = 25
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 25)
        if (vercettiVariationState = 2)
        or (vercettiVariationState = 3)
            currentWardrobeClothingNumber = 24
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 24)
        if (casualVariationState = 2)
        or (casualVariationState = 3)
            currentWardrobeClothingNumber = 23
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 23)
        if (countryVariationState = 2)
        or (countryVariationState = 3)
            currentWardrobeClothingNumber = 22
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 22)
        if (soireeVariationState = 2)
        or (soireeVariationState = 3)
            currentWardrobeClothingNumber = 21
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 21)
        if (streetVariationState = 2)
        or (streetVariationState = 3)
            currentWardrobeClothingNumber = 20
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 20)
        if (blackTracksuitVariationState = 1)
        or (blackTracksuitVariationState = 3)
            currentWardrobeClothingNumber = 19
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 19)
        if (blackTracksuitVariationState = 1)
        or (blackTracksuitVariationState = 3)
            currentWardrobeClothingNumber = 18
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 18)
        if (vercettiVariationState = 1)
        or (vercettiVariationState = 3)
            currentWardrobeClothingNumber = 17
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 17)
        if (casualVariationState = 1)
        or (casualVariationState = 3)
            currentWardrobeClothingNumber = 16
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 16)
        if (countryVariationState = 1)
        or (countryVariationState = 3)
            currentWardrobeClothingNumber = 15
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 15)
        if (soireeVariationState = 1)
        or (soireeVariationState = 3)
            currentWardrobeClothingNumber = 14
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 14)
        if (streetVariationState = 1)
        or (streetVariationState = 3)
            currentWardrobeClothingNumber = 13
            return
        endif
    endif
    if (currentWardrobeClothingNumber >= 13)
    and (created_final_shirt = 1)
        currentWardrobeClothingNumber = 12
        return
    endif
    if (currentWardrobeClothingNumber >= 12)
    and (clothes13_created = 1)
        currentWardrobeClothingNumber = 11
        return
    endif
    if (currentWardrobeClothingNumber >= 11)
    and (clothes6_created = 1)
        currentWardrobeClothingNumber = 10
        return
    endif
    if (currentWardrobeClothingNumber >= 10)
    and (clothes5_created = 1)
        currentWardrobeClothingNumber = 9
        return
    endif
    if (currentWardrobeClothingNumber >= 9)
    and (clothes3_created = 1)
        currentWardrobeClothingNumber = 8
        return
    endif
    if (currentWardrobeClothingNumber >= 8)
    and (clothes9_created = 1)
        currentWardrobeClothingNumber = 7
        return
    endif
    if (currentWardrobeClothingNumber >= 7)
    and (clothes9_created = 1)
        currentWardrobeClothingNumber = 6
        return
    endif
    if (currentWardrobeClothingNumber >= 6)
    and (clothes8_created = 1)
        currentWardrobeClothingNumber = 5
        return
    endif
    if (currentWardrobeClothingNumber >= 5)
    and (clothes7_created = 1)
        currentWardrobeClothingNumber = 4
        return
    endif
    if (currentWardrobeClothingNumber >= 4)
    and (clothes4_created = 1)
        currentWardrobeClothingNumber = 3
        return
    endif
    if (currentWardrobeClothingNumber >= 3)
    and (clothes2_created = 1)
        currentWardrobeClothingNumber = 2
        return
    endif

    currentWardrobeClothingNumber = 1
    return

wardrobe_before_change_clothes_next:
    if (currentWardrobeClothingNumber = 1)
    and (clothes2_created = 1)
        currentWardrobeClothingNumber = 2
        return
    endif
    if (currentWardrobeClothingNumber <= 2)
    and (clothes4_created = 1)
        currentWardrobeClothingNumber = 3
        return
    endif
    if (currentWardrobeClothingNumber <= 3)
    and (clothes7_created = 1)
        currentWardrobeClothingNumber = 4
        return
    endif
    if (currentWardrobeClothingNumber <= 4)
    and (clothes8_created = 1)
        currentWardrobeClothingNumber = 5
        return
    endif
    if (currentWardrobeClothingNumber <= 5)
    and (clothes9_created = 1)
        currentWardrobeClothingNumber = 6
        return
    endif
    if (currentWardrobeClothingNumber <= 6)
    and (clothes9_created = 1)
        currentWardrobeClothingNumber = 7
        return
    endif
    if (currentWardrobeClothingNumber <= 7)
    and (clothes3_created = 1)
        currentWardrobeClothingNumber = 8
        return
    endif
    if (currentWardrobeClothingNumber <= 8)
    and (clothes5_created = 1)
        currentWardrobeClothingNumber = 9
        return
    endif
    if (currentWardrobeClothingNumber <= 9)
    and (clothes6_created = 1)
        currentWardrobeClothingNumber = 10
        return
    endif
    if (currentWardrobeClothingNumber <= 10)
    and (clothes13_created = 1)
        currentWardrobeClothingNumber = 11
        return
    endif
    if (currentWardrobeClothingNumber <= 11)
    and (created_final_shirt = 1)
        currentWardrobeClothingNumber = 12
        return
    endif
    if (currentWardrobeClothingNumber <= 12)
        if (streetVariationState = 1)
        or (streetVariationState = 3)
            currentWardrobeClothingNumber = 13
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 13)
        if (soireeVariationState = 1)
        or (soireeVariationState = 3)
            currentWardrobeClothingNumber = 14
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 14)
        if (countryVariationState = 1)
        or (countryVariationState = 3)
            currentWardrobeClothingNumber = 15
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 15)
        if (casualVariationState = 1)
        or (casualVariationState = 3)
            currentWardrobeClothingNumber = 16
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 16)
        if (vercettiVariationState = 1)
        or (vercettiVariationState = 3)
            currentWardrobeClothingNumber = 17
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 17)
        if (blackTracksuitVariationState = 1)
        or (blackTracksuitVariationState = 3)
            currentWardrobeClothingNumber = 18
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 18)
        if (redTracksuitVariationState = 1)
        or (redTracksuitVariationState = 3)
            currentWardrobeClothingNumber = 19
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 19)
        if (streetVariationState = 2)
        or (streetVariationState = 3)
            currentWardrobeClothingNumber = 20
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 20)
        if (soireeVariationState = 2)
        or (soireeVariationState = 3)
            currentWardrobeClothingNumber = 21
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 21)
        if (countryVariationState = 2)
        or (countryVariationState = 3)
            currentWardrobeClothingNumber = 22
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 22)
        if (casualVariationState = 2)
        or (casualVariationState = 3)
            currentWardrobeClothingNumber = 23
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 23)
        if (vercettiVariationState = 2)
        or (vercettiVariationState = 3)
            currentWardrobeClothingNumber = 24
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 24)
        if (blackTracksuitVariationState = 2)
        or (blackTracksuitVariationState = 3)
            currentWardrobeClothingNumber = 25
            return
        endif
    endif
    if (currentWardrobeClothingNumber <= 25)
        if (redTracksuitVariationState = 2)
        or (redTracksuitVariationState = 3)
            currentWardrobeClothingNumber = 26
            return
        endif
    endif

    currentWardrobeClothingNumber = 1
    return

wardrobe_change_clothes:
    currentVariation = 0

    // Street
    if (currentWardrobeClothingNumber = 1)
        UNDRESS_CHAR scplayer player
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Soiree
    if (currentWardrobeClothingNumber = 2)
        UNDRESS_CHAR scplayer PLAYER2
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Country Club
    if (currentWardrobeClothingNumber = 3)
        UNDRESS_CHAR scplayer PLAYER4
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Casual
    if (currentWardrobeClothingNumber = 4)
        UNDRESS_CHAR scplayer PLAYER8
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Mr Vercetti
    if (currentWardrobeClothingNumber = 5)
        UNDRESS_CHAR scplayer PLAYER9
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Black tracksuit
    if (currentWardrobeClothingNumber = 6)
        UNDRESS_CHAR scplayer PLAY10
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Red tracksuit
    if (currentWardrobeClothingNumber = 7)
        UNDRESS_CHAR scplayer PLAY11
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Coveralls
    if (currentWardrobeClothingNumber = 8)
        UNDRESS_CHAR scplayer PLAYER3
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Havana
    if (currentWardrobeClothingNumber = 9)
        UNDRESS_CHAR scplayer PLAYER5
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Cop
    if (currentWardrobeClothingNumber = 10)
        UNDRESS_CHAR scplayer PLAYER6
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Bank Job
    if (currentWardrobeClothingNumber = 11)
        UNDRESS_CHAR scplayer PLAYER7
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Frankie
    if (currentWardrobeClothingNumber = 12)
        UNDRESS_CHAR scplayer PLAY12
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    currentVariation = 1

    // Street
    if (currentWardrobeClothingNumber = 13)
        UNDRESS_CHAR scplayer PLAYER
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Soiree
    if (currentWardrobeClothingNumber = 14)
        UNDRESS_CHAR scplayer PLAYER2
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif


    // Country Club
    if (currentWardrobeClothingNumber = 15)
        UNDRESS_CHAR scplayer PLAYER4
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Casual
    if (currentWardrobeClothingNumber = 16)
        UNDRESS_CHAR scplayer PLAYER8
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Mr Vercetti
    if (currentWardrobeClothingNumber = 17)
        UNDRESS_CHAR scplayer PLAYER9
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Black tracksuit
    if (currentWardrobeClothingNumber = 18)
        UNDRESS_CHAR scplayer PLAY10
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Red tracksuit
    if (currentWardrobeClothingNumber = 19)
        UNDRESS_CHAR scplayer PLAY11
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    currentVariation = 2

    // Street
    if (currentWardrobeClothingNumber = 20)
        UNDRESS_CHAR scplayer PLAYER
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Soiree
    if (currentWardrobeClothingNumber = 21)
        UNDRESS_CHAR scplayer PLAYER2
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif


    // Country Club
    if (currentWardrobeClothingNumber = 22)
        UNDRESS_CHAR scplayer PLAYER4
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Casual
    if (currentWardrobeClothingNumber = 23)
        UNDRESS_CHAR scplayer PLAYER8
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Mr Vercetti
    if (currentWardrobeClothingNumber = 24)
        UNDRESS_CHAR scplayer PLAYER9
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Black tracksuit
    if (currentWardrobeClothingNumber = 25)
        UNDRESS_CHAR scplayer PLAY10
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    // Red tracksuit
    if (currentWardrobeClothingNumber = 26)
        UNDRESS_CHAR scplayer PLAY11
        LOAD_ALL_MODELS_NOW
        DRESS_CHAR scplayer

        SET_CHAR_CLOTHING_VARIATION scplayer currentVariation

        GOSUB wardrobe_wait
        RETURN
    endif

    return

wardrobe_put_on_clothes:
    if (currentWardrobeClothingNumber = storedWardrobeClothingNumber)
        RETURN
    endif

    while (IS_BUTTON_PRESSED PAD1 TRIANGLE) // VEHICLE_ENTER_EXIT
        WAIT 0
    endwhile

    GET_PLAYER_COORDINATES player1 playera_x playera_y playera_z
    ADD_ONE_OFF_SOUND playera_x playera_y playera_z SOUND_AMMUNATION_BUY_WEAPON

    storedWardrobeClothingNumber = currentWardrobeClothingNumber
    storedVariation = currentVariation

    RETURN

leave_wardrobe:
    if (storedWardrobeClothingNumber = 1)
    or (storedWardrobeClothingNumber = 13)
    or (storedWardrobeClothingNumber = 20)
        UNDRESS_CHAR scplayer PLAYER
    endif
    if (storedWardrobeClothingNumber = 2)
    or (storedWardrobeClothingNumber = 14)
    or (storedWardrobeClothingNumber = 21)
        UNDRESS_CHAR scplayer PLAYER2
    endif
    if (storedWardrobeClothingNumber = 3)
    or (storedWardrobeClothingNumber = 15)
    or (storedWardrobeClothingNumber = 22)
        UNDRESS_CHAR scplayer PLAYER4
    endif
    if (storedWardrobeClothingNumber = 4)
    or (storedWardrobeClothingNumber = 16)
    or (storedWardrobeClothingNumber = 23)
        UNDRESS_CHAR scplayer PLAYER8
    endif
    if (storedWardrobeClothingNumber = 5)
    or (storedWardrobeClothingNumber = 17)
    or (storedWardrobeClothingNumber = 24)
        UNDRESS_CHAR scplayer PLAYER9
    endif
    if (storedWardrobeClothingNumber = 6)
    or (storedWardrobeClothingNumber = 18)
    or (storedWardrobeClothingNumber = 25)
        UNDRESS_CHAR scplayer PLAY10
    endif
    if (storedWardrobeClothingNumber = 7)
    or (storedWardrobeClothingNumber = 19)
    or (storedWardrobeClothingNumber = 26)
        UNDRESS_CHAR scplayer PLAY11
    endif
    if (storedWardrobeClothingNumber = 8)
        UNDRESS_CHAR scplayer PLAYER3
    endif
    if (storedWardrobeClothingNumber = 9)
        UNDRESS_CHAR scplayer PLAYER5
    endif
    if (storedWardrobeClothingNumber = 10)
        UNDRESS_CHAR scplayer PLAYER6
    endif
    if (storedWardrobeClothingNumber = 11)
        UNDRESS_CHAR scplayer PLAYER7
    endif
    if (storedWardrobeClothingNumber = 12)
        UNDRESS_CHAR scplayer PLAY12
    endif
    LOAD_ALL_MODELS_NOW
    DRESS_CHAR scplayer

    SET_CHAR_CLOTHING_VARIATION scplayer storedVariation

    RESTORE_CAMERA_JUMPCUT
    SET_CAMERA_BEHIND_PLAYER

    CLEAR_HELP

    SET_PLAYER_CONTROL 0 TRUE

    if (currentPickupInUse = 1)
        CREATE_CLOTHES_PICKUP 226.4 -1265.6 20.1 1 clothes_pickup1
    endif
    if (currentPickupInUse = 2)
        CREATE_CLOTHES_PICKUP -384.5 -591.9 25.3 1 mansion_clothes
    endif
    if (currentPickupInUse = 3)
        CREATE_CLOTHES_PICKUP -820.2 1364.1 66.4 1 safehouse_clothes2
    endif
    currentPickupInUse = 0

    wardrobePosX = 0.0
    wardrobePosY = 0.0
    wardrobePosZ = 0.0
    wardrobeHeading = 0.0
    storedVariation = 0
    currentVariation = 0
    storedWardrobeClothingNumber = 0
    currentWardrobeClothingNumber = 1

    GOTO wardrobe_loop

wardrobe_wait:
    TIMERA = 0
    while (TIMERA < 300)
        WAIT 0
        GOSUB wardrobe_display_info
    endwhile

    RETURN
}