#include <M5Unified.h>
#include <WiFi.h>
extern "C" {
  #include "esp_wifi.h"
}
#include "esp32-hal-bt.h"
#include <math.h>
#include <Preferences.h>
#include <LittleFS.h>

#define COUNT(arr) (int)(sizeof(arr) / sizeof(arr[0]))

// fallback timing — replaced at boot once we know the actual WAV duration
uint32_t SPIN_MS   = 600;
uint32_t REVEAL_MS = 3900;
uint32_t ANIM_MS   = 4500;
const uint32_t FRAME_MS      = 40;
const uint32_t MSG_HOLD      = 800;
const uint32_t MSG_ANIM_MS   = 2000;
const uint32_t MSG_SPIN_MS   = 250;
const uint32_t MSG_REVEAL_MS = MSG_ANIM_MS - MSG_SPIN_MS;
const uint32_t STATUS_HOLD_MS = 5000;
const uint32_t SLEEP_DUR      = 3000;
const uint32_t DEBOUNCE       = 350;
const uint32_t INACTIVITY_MS  = 120000;

const int MAX_LINES = 12;
const int LINE_GAP  = 2;

const uint8_t BRIGHTNESS = 10;
// HIGH = stay on, LOW = cut power (self-latch circuit)
const int     PIN_HOLD   = 4;

const char* USER_NAME  = "Proxy Denis";
const char* SLEEP_TEXT = "2.718281";

// dark teal (~#1A3535 in RGB565)
const uint16_t COL_BG   = 0x19A6;
const uint16_t COL_TEXT = TFT_WHITE;

const char* ALPHABET =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789"
  "!@#$%^&*()-_=+[]{};:,.?/|";
const int ALPHA_LEN = 26 + 26 + 10 + 22;


const char* singles[] = {
  "Play rock paper scissors with the next person you meet. If you lose, tell them something true.",
  "Count every door you pass through today. At the last one, pause.",
  "Write your name on something that will not hold it for long.",
  "Tell someone a story about someone else as if it were your own.",
  "Before you sleep, list three things you could not have predicted this morning.",
  "Find something abandoned. Leave it somewhere it will be found.",
  "Say the name of someone you miss, aloud, once.",
  "Spend one minute in complete stillness. Do not explain why.",
  "Look at your hands. Remember what they have held.",
  "Choose a number between one and ten. Tell no one.",
  "Eat the next meal you have in silence. Notice everything.",
  "Take a route you have never taken before. Do not record it.",
  "Read aloud the nearest sentence to hand. Consider it carefully.",
  "Give away something you were not planning to give away today.",
  "Apologize to someone. Do not tell them why.",
  "Count the windows visible from where you stand. Remember this number.",
  "The next person who makes you laugh — remember their name.",
  "Close your eyes for thirty seconds. Open them. Begin again.",
  "Play rock paper scissors with the third person you meet and play rock. If you win, pull out 59 strands of their hair. Then, apply seafood-cream pasta sauce with mealworms fed on styrofoam to it three times, and eat it with a fork.",
  "Put 3 needles in your neighbor's birthday cake.",
  "Make risotto out of sewer water, and feed it to the person next door.",
  "Rip out the spine of someone who wronged the Index.",
  "Kill the painting you've drawn.",
  "Move a unicorn plushie to a park.",
  "See green from a white wall.",
  "Stand on any three-way intersection at 3:38 tomorrow, look to the east, and wave seven times.",
  "Exchange the left leg of the fourteenth person you come across with the right leg of the twenty-sixth.",
  "When you see a person on a three-way intersection waving their hand seven times, follow them home.",
  "Kill all the doors of the ceiling.",
  "Consume your vocal cords, boiled for 14 seconds in simmering salt water, as part of your dinnertime meal.",
  "Climb to the rooftop of a building 3 stories or higher and wave your hand while looking down for 1 minute.",
  "Enter as the third customer of the day to a restaurant that sells chickens, and leave last.",
  "Start a game of hide-and-seek with at least three people, then return home as the 'it.'",
  "Read six books, then visit the District that appears in the last book read. No time limit.",
  "Kill the liar within the alley within alleys.",
  "Pack a lunchbox and consume it on top of a trash can in the streets of District 11 at 1 PM today.",
  "Bake dacquoise while the hour hand rests between 7 and 8, and eat it while watching a movie.",
  "Initiate a game of Never Have I Ever with the first five people you encounter. When one folds a finger, break it.",
  "Neatly clip the nails of the sixty-second person you come across.",
  "Pet quadrupedal animals five times.",
  "Spin a wheel and throw a cake at the person determined by the result.",
  "Consume eight crabs stored at room temperature and ripe persimmon at once.",
  "At the railing on the roof of a building, shout out the name of the person you dislike, then jump off. The height of the building does not matter.",
  "After a meal, discard all dishes that were used to serve it.",
  "On the morning after receiving the Prescript, drink three cups of water as soon as you get up.",
  "Race against residents in the same building as you to District 7. Measure the distance every twenty-three minutes and disqualify the one farthest from the destination.",
  "Within three days, knit a scarf with a butterfly pattern.",
  "Dial any number. Give a New Year's greeting and words of blessing to whoever receives the call.",
  "When hungry, consume a cheeseburger with added onion.",
  "Fold 39 paper cranes and throw them from the rooftop.",
  "At work, cut the ear of the first person to fulminate against you.",
  "When your eyes meet another person's, nod at them.",
  "Return to your home this instant. You may leave once a dog barks in front of your house.",
  "Wear light green clothing and take 10 steps in a triangle-shaped alley.",
  "When penetrating the lungs with a stiletto, lay vertical the end, insert up to the wick.",
  "Destroy the sound, crush flat the thought.",
  "Sleep for a total of 800 hours per day.",
  "Drink a liter of milk. Warm up before you go play.",
  "Only read, write, or pull the trigger with your right hand.",
  "In 30 minutes, find a groom or bride — bonus if brunette. In 90 hours, spill their insides. Paint the room picturesque.",
  "In 400000 meters, turn right.",
  "Do not go home until you finish reading the value of e.",
  "Grind upon dawn's rail.",
  "Shove an entire orange inside a grapefruit, then hit it with a hammer repeatedly while meowing at 9:34 on a Friday night.",
  "Untie every shoelace in your household. You may re-tie them after you are next greeted by name.",
  "Open and close a north-facing window 13 times, then close the curtains for the rest of the week.",
  "Walk to the nearest intersection and head south for 17 blocks. Within 13 days, descend to the lowest floor of the building you arrive at. If you meet a man there, shake his hand.",
  "Start drawing, and do nothing else until you're done. Print it out and eat it.",
  "Gut a fish and place one ounce of the meat on your neighbor's pillow, along with the bones. Consume the rest raw without exception.",
  "Drink eight full cups of water within a twenty-four hour period.",
  "Immediately locate and play the nearest physical copy of a video game. If you cannot complete it within 24 hours, dye your hair red.",
  "Walk up to the front door of a random house on your street. If they have a door camera, perform a magic trick while staring into it, then leave. Do not make a sound.",
  "The next time you lose in a card game, eat every card in the winner's hand. If they attempt to stop you, also eat their hand.",
  "Solve 23 nonogram puzzles in a row.",
  "Go out for a walk with your left shoe untied, and when someone notices, tell them that their right shoe is untied.",
  "At the crossroads, do not turn left.",
  "Make out an image of something in the trees, then fear it for 3 days.",
  "Take a Proxy out to a nice restaurant of their choosing. If you both order the same thing, leave without paying.",
  "Recite the value of e.",
  "At daybreak, noon, and sunset, go outside and bask in the sunlight, take a picture with the sun, and wave at the sun.",
  "Fill a blender with even amounts of ketchup, hummus and molasses. Blend for 7 and a half minutes, and chug the result straight from the appliance.",
  "Purchase tickets for a flight over 3 hours long. Sit in the middle seat between two strangers. Quietly smack your lips every 5 to 10 minutes for the duration of the flight.",
  "Do 17 cartwheels immediately.",
  "Eat 40 peas. Be sure to peel each one and separate the two halves.",
  "Alter your body. Mirror the Fixer lost to the Library.",
  "Show this Prescript to your nearest Proxy. They will understand what it means.",
  "For the next 12 hours, only walk with your feet faced north. When the time ends, repeat the Prescript, walking only using your heels.",
  "Remember to aim for the heart.",
  "Read out the entirety of 'Prayer For Loving Sorrow'.",
  "Watch a Tortoise Spiral reach its end.",
  "Wordlessly hug someone who looks like they need it the least.",
  "Invent a new gender-neutral alternative to niece or nephew within the next ten minutes. Consider this Prescript failed if the nearest person doesn't like it.",
  "Listen to the sound of the waves for 4 hours.",
  "Draw the muse that lives in your head.",
  "Spin around counterclockwise until you see triple, then stab the fake illusions until they bleed shadows.",
  "On the third day of the next month, walk past 3 people who are holding hands, tap their shoulders 3 times and offer a handshake. If they don't comply, start over.",
  "While carrying a pile of books, crash into someone.",
  "Challenge every right-handed person who is not an Index member that you encounter to a staring contest, until you lose. Give meaning to the person who beat you.",
  "Count the teeth of the next 7 people you meet. If any have fewer than 32, replace the missing ones with your own.",
  "Next time you are in a life-threatening situation, take a 20 minute break. Act as if nothing fazes you for the duration.",
  "Go to a graveyard and lay across the third grave you see. Before 30 minutes have passed, make sure to water your dreams.",
  "Let her voice reach you. Do as she says.",
  "Let her voice reach you. Deny her will.",
  "Bring your daughter back home.",
  "Tell your nearest neighbor about the future. Ensure it comes to pass.",
};

const char* times[] = {
  "Tomorrow, ",          "By midnight, ",       "Within 3 hours, ",
  "Before dawn, ",       "At noon, ",           "This evening, ",
  "Before you sleep, ",  "Immediately, ",       "By the next dawn, ",
  "Within the hour, ",
  "Next week, ",
  "As soon as possible, ",
  "At midnight, ",
  "During the Night in the Backstreets, ",
  "Next time you cross paths with your best friend, ",
  "When your eyes meet with a stranger, ",
  "When you are next given a Prescript, ",
  "Next time you encounter someone, ",
  "Until someone stops you, ",
  "The next time you are in a life-threatening situation, ",
  "Before the next snowfall, ",
  "Before the next time it rains, ",
  "The next time you fall in love, ",
  "During your next meal, ",
  "After you spot a rainbow, ",
  "When at work, ",
  "When in a classroom, ",
  "On the morning after receiving the Prescript, ",
  "On the evening after receiving the Prescript, ",
  "Within 72 hours, ",
  "Next Sunday, ",
  "Next Monday, ",
};

const char* actions[] = {
  "speak to ",            "give something to ",   "observe ",
  "make something for ",  "listen to ",           "offer your name to ",
  "share a meal with ",   "ask one question of ", "walk alongside ",
  "be honest with ",      "apologize to ",        "express gratitude to ",
  "sit in silence with ", "tell a truth to ",
  "help ",                             "hug ",
  "trade a precious item with ",       "gouge out the left eye of ",
  "gouge out the right eye of ",       "exchange vows with ",
  "engage in a fistfight with ",       "rip out the spine of ",
  "crush the skull of ",               "take a walk with ",
  "hold hands with ",                  "buy dinner for ",
  "eat a meal with ",                  "head out on an extended vacation with ",
  "ask what is the name of ",          "dance with ",
  "send a love letter to ",            "send a handwritten confession letter to ",
};

const char* targets[] = {
  "a stranger",                  "someone you trust",          "your closest friend",
  "yourself",                    "the next person who speaks to you",
  "someone who owes you nothing","an old acquaintance",
  "someone you have wronged",    "someone who has wronged you",
  "a child",                     "an elder",
  "someone you have not spoken to in a long time",
  "your next-door neighbor",
  "your nearest coworker",
  "your nearest Index member",
  "a total stranger",
  "your best friend",
  "the face in the mirror",
  "your mother",
  "your father",
  "the person you hate the most",
  "the person you love the most",
  "your next-door neighbors",
  "a Color Fixer",
  "a Grade 5 Fixer",
  "a Grade 8 Fixer",
  "the one you couldn't kill",
  "someone who wronged you",
  "your doctor",
  "one you would consider a friend",
  "one you would consider a friend, but they do not",
  "someone that considers you a friend, but you do not",
  "residents that live in the same building as you",
  "the leader of the closest Syndicate",
  "your dreams",
};

const char* postscripts[] = {
  " Do not explain yourself.",      " Ask for nothing in return.",
  " Do not speak of it again.",     " Keep no record of it.",
  " Forget it by tomorrow.",        " Remember it exactly.",
  " Let it be enough.",             " Do not hesitate.",
  " You have no reason. That is fine.",
  " Make no mention of this directive.",
  " No time limit.",
  " You must be wearing black.",
  " Remember to wear glasses when you do.",
  " Leave no witnesses.",
  " Return home as soon as you can.",
  " Close the door immediately.",
  " Be sure to keep an eye behind you.",
  " Take a break when you're done.",
  " The next time you receive a Prescript, remember you have to disobey it.",
  " You must hold your breath for the duration.",
  " You must not blink.",
  " Don't let yourself be caught.",
  " Shower immediately afterwards.",
  " Once this Prescript is done, you may consider yourself a Proselyte of the Index.",
  " Once this Prescript is done, you will be promoted to Proxy. Congratulations.",
  " Once this Prescript is done, you will be promoted to Messenger. Congratulations.",
  " Time limit: before you next spot three crows on the same electric wire.",
  " Ask yourself if it was worth it.",
  " Invite a friend to do the same.",
  " Afterwards, board the next WARP train.",
  " Once you're done, drink a cup of sewage.",
};

const char* transitions[] = { "Then,", "Afterward,", "After that," };


const char* ny_locA[] = {
  "the beach",                           "a store",
  "a house",                             "an apartment complex",
  "the ocean",                           "a hotel",
  "a motel",                             "a place you cherish",
  "a place you keep your family",        "a restaurant",
  "a place to eat",                      "a theater",
  "the library",                         "a school",
  "a factory",                           "a backstreet alleyway",
  "a nest",                              "an empty field",
  "a graveyard",                         "a basement",
  "an alley",                            "a parking lot",
  "an abandoned lot",                    "an office",
  "a workshop",                          "a place no one checks twice",
  "a place people pass through quickly", "a place that won't last forever",
  "a place that smells like dust",       "a place that echoes",
  "a place you were told not to enter",  "a public restroom",
  "a train platform",                    "a lighthouse",
  "a secret location only you know",     "a place you call home",
  "a place they call home",              "a place you'd hide a body",
  "an office building",                  "a Syndicate den",
  "a bakery",                            "a convenience store",
  "a murky alleyway",                    "your workplace",
};

const char* ny_pId[] = {
  "ugly",              "beautiful",     "handsome",      "cute",
  "pretty",            "plain",         "attractive",    "unattractive",
  "sexually-attractive","young",        "old",           "middle-aged",
  "elderly",           "tall",          "short",         "thin",
  "fat",               "muscular",      "pale",          "dark",
  "scarred",           "freckled",      "blonde",        "bald",
  "kind",              "mean",          "selfish",       "generous",
  "friendly",          "hostile",       "wise",          "foolish",
  "intelligent",       "confident",     "shy",           "calm",
  "nervous",           "aggressive",    "passive",       "healthy",
  "sickly",
};

const char* ny_pId2[] = {
  "ugliest",         "prettiest",        "handsomest",       "cutest",
  "creepiest",       "kindest",          "meanest",          "cruelest",
  "warmest",         "smartest",         "dumbest",          "wisest",
  "strongest",       "weakest",          "bravest",          "boldest",
  "shyest",          "loudest",          "funniest",         "weirdest",
  "most-delusional", "most-violent",     "most-predatory",   "most-manipulative",
  "most-sadistic",   "most-ruthless",    "most-merciless",   "most-terrifying",
  "most-unstoppable",
};

const char* ny_pType[] = {
  "person",       "stranger",     "child",        "teenager",
  "adult",        "elder",        "neighbor",     "family member",
  "sibling",      "parent",       "teacher",      "student",
  "priest",       "Fixer",        "official",     "doctor",
  "nurse",        "butcher",      "baker",        "guard",
  "villager",     "homeless man", "drunk man",    "Index member",
  "Syndicate member",
};

const char* ny_cloth[] = {
  "hat",    "cap",     "hood",    "hoodie",
  "cloak",  "coat",    "jacket",  "vest",
  "shirt",  "t-shirt", "dress",   "pants",
  "shoes",  "boots",   "gloves",  "scarf",
  "uniform",
};

const char* ny_mat[] = {
  "cloth",    "leather",    "wool",    "cotton",
  "silk",     "velvet",     "new",     "old",
  "worn",     "torn",       "ripped",  "clean",
  "dirty",    "wet",        "bloodstained", "burnt",
  "blue",     "black",      "red",     "brown",
  "yellow",   "pink",       "white",   "grey",
  "green",    "cyan",       "teal",    "purple",
  "orange",   "gold",       "silver",  "crimson",
  "scarlet",
};

const char* ny_topic[] = {
  "love",       "life",         "death",       "fear",
  "regret",     "hope",         "loneliness",  "trust",
  "guilt",      "desire",       "memories",    "dreams",
  "gaming",     "the city",     "Hatsune Miku","birds",
  "smoke",      "crypto",       "money",       "identity",
  "meaning",    "power",        "justice",     "family",
  "friendship", "violence",     "the Fixers",  "who you really are",
};

const char* ny_games[] = {
  "Patty Cake",                   "Never Have I Ever",
  "Tag",                          "Hide and Seek",
  "Chess",                        "Dominoes",
  "Twister",                      "Rock Paper Scissors",
  "a game you were warned about", "a game no one explains",
  "a game that doesn't end",
};

const char* ny_objcs[] = {
  "pen and paper",  "apples",           "an empty bottle",
  "duct tape",      "charcoal",         "a wooden chair",
  "dice",           "a key",            "a backpack",
  "a flashlight",   "a mirror",         "a notebook",
  "an umbrella",    "a hammer",         "a shovel",
  "a fishing rod",  "rope",             "a knife",
  "a broken clock", "a photo",          "salt",
  "a candle",       "a matchbox",
};

const char* ny_v2[] = {
  "comforted", "followed",  "ignored",  "watched",
  "questioned","stabbed",   "hugged",   "kissed",
  "punched",   "shot",      "killed",
};


const char* nc_act2[] = {
  "eat bitter.",
  "jump from a roof. The height does not matter.",
  "jump from a roof. It must be at least 30 meters tall.",
  "fetch a cup of water.",
  "drink a cup of sewage.",
  "grab a coffee with a friend.",
  "shoot yourself.",
  "help your nearest neighbor cross the street.",
  "engage in a fistfight with the person you hate most.",
  "rip out the spine of your nearest coworker.",
  "crush the skull of a total stranger.",
  "hold hands with your best friend.",
  "send a love letter to the person you love the most.",
  "send a handwritten confession letter to a total stranger.",
  "immigrate to a different district.",
  "look at yourself in the mirror.",
  "give your nearest neighbor a manicure.",
  "wave to the face in the mirror 7 times a day.",
  "fight with the person you hate most to the death.",
  "bake cookies and give them to your nearest neighbor.",
  "take 40 steps in an alley.",
  "head to a hotel with the first stranger you see.",
  "run for 3 hours facing north. If anyone stands in your way, cut them down.",
  "spill your blood into the nearest toilet.",
  "tell your best friend your darkest secret.",
  "buy tickets to a WARP train ride. They must be Economic Class.",
  "buy tickets to a WARP train ride. They must be First Class.",
  "board the next WARP train.",
  "wish a happy new year to a couple walking a dog.",
  "head north for 12 blocks.",
  "start a bug band.",
  "run into traffic.",
  "steal copper wiring from your neighbor's house.",
  "immediately go and knock on your neighbor's door. If they answer, exchange Prescripts, then follow theirs to the letter.",
  "destroy fate.",
  "convince someone else to fail this Prescript.",
  "recite the value of e.",
  "go to sleep, and do not wake until the following morning.",
  "seek that which will fill your heart.",
  "apply for a job.",
  "rip the blinds.",
  "weep from joy, sorrow, and fear.",
  "give yourself a secret name.",
  "drink from a puddle in the street.",
  "pet the nearest dog, even if it means breaking into somewhere.",
  "brew a cup of oolong tea.",
  "brew a cup of black tea.",
  "spend 23 hours as usual. In the 24th, engage in something you've never done before.",
  "insult a member of the Thumb, and blame it on someone else.",
  "insult a member of the Middle, and blame it on someone else.",
  "learn how to crochet with your fingers.",
  "paint the sky as you see it in the moment.",
  "do the macarena.",
  "do a backflip.",
  "speak only in numbers until you are told to stop.",
  "take a bubble bath.",
  "search for a District that doesn't exist.",
  "cross something off your bucket list.",
  "apply seafood-cream pasta sauce with mealworms fed on styrofoam to your food three times. Eat it with a fork.",
  "get the name of the next of kin from the next opponent you defeat. If they do not have one, behead them.",
  "play tag with your nearest neighbor, and make sure you win.",
  "leave all the houselights on.",
  "board a WARP train with your nearest coworker.",
};

const char* nc_act1[] = {
  "Fix your posture, ",
  "Forget your own reflection, ",
  "Take a selfie, ",
  "Turn off your nearest computer, ",
  "Take a shower, ",
  "Ask for a different Prescript, ",
  "Get banned from your most used social media, ",
  "Pack a lunchbox, ",
  "Pack for a short trip, ",
  "Pack for a long trip, ",
  "Flip a coin until you get heads, ",
  "Flip a coin until you get tails, ",
  "Walk to the nearest intersection, ",
  "Reap what you've sowed, ",
  "Order a rice dish, count all of the individual grains, ",
  "Walk into the nearest bookstore, pick the first book you see, read it cover to cover, ",
  "Eat only red-colored foods for 3 days, ",
  "Eat nothing but pasta for a week, ",
  "Buy a food magazine and underline every instance of the word \"pepper\", ",
  "Go to the nearest fast food restaurant, order the first item your eyes fall on, ",
  "Find yourself in the creases of the couch, ",
  "During the Night in the Backstreets, kill 10 or more Sweepers, ",
  "Picture yourself in your favorite Association's uniform, ",
  "Record a bird for 15 minutes, ",
  "Ask someone about their 'ideal', ",
  "Break the first clock you see, ",
  "Go to the closest school, ",
  "Clap without a sound, ",
  "Name five things you can't see, ",
  "Run late to an important event with a piece of toast in your mouth, ",
  "Express an opinion on social media you disagree with, ",
  "Do a ten-pull on your latest played gacha game, ",
  "Order something different from the usual in your most frequented restaurant, ",
  "Brew a cup of green tea, ",
  "Dance to the melody in your head, ",
  "List 5 positive things about yourself in a napkin, ",
  "Read the entirety of the first random article you draw on Wikipedia, ",
  "Play a game you have not opened in years, ",
  "Rip off the arms of your nearest coworker, ",
  "Rip off the legs of your nearest coworker, ",
  "Look north for 30 minutes without ever moving your head away, ",
  "Burn the last gift you received, ",
  "Bring the head of your nearest neighbor to a total stranger, ",
};

const char* nc_markers[] = {
  "then ",
  "and immediately afterwards, ",
  "but before you do that, ",
  "and once you do, ",
  "pretend nothing happened, and ",
  "and ",
  "let it sink in, and ",
  "but if you can't, ",
};


M5Canvas   canvas(&M5.Display);
Preferences prefs;

enum class Mode { BOOT, ANIMATING, READING, CLEARING, CLEARING_HOLD, MESSAGE, STATUS, STATUS_HOLD, SLEEPING };
Mode mode = Mode::BOOT;

char        prescriptLines[MAX_LINES][64];
const char* prescriptPtrs[MAX_LINES];
int         lineCount = 0;

char target[64];

char        statusLines[4][64];
const char* statusPtrs[4];

uint32_t stateStart    = 0;
uint32_t lastFrame     = 0;
bool     holding       = false;
uint32_t totalClear    = 0;
uint32_t btnLockout    = 0;
uint32_t lastActivity  = 0;
uint32_t btnPWRStart   = 0;

uint8_t* wavReveal   = nullptr;
size_t   wavRevealSz = 0;
uint8_t* wavClear    = nullptr;
size_t   wavClearSz  = 0;
uint8_t* bootImgBuf  = nullptr;
size_t   bootImgSz   = 0;


static void loadWav(const char* path, uint8_t** buf, size_t* sz) {
  File f = LittleFS.open(path);
  if (!f || f.isDirectory()) return;
  size_t fileSize = f.size();
  if (fileSize == 0) { f.close(); return; }
  uint8_t* mem = (uint8_t*)malloc(fileSize);
  if (mem) {
    f.read(mem, fileSize);
    *buf = mem;
    *sz  = fileSize;
  }
  f.close();
}

// IHDR width/height sit at bytes 16–23, big-endian
static bool getPngSize(const uint8_t* buf, size_t sz, uint32_t* w, uint32_t* h) {
  if (sz < 24 || memcmp(buf, "\x89PNG\r\n\x1a\n", 8) != 0) return false;
  *w = ((uint32_t)buf[16] << 24) | ((uint32_t)buf[17] << 16) | ((uint32_t)buf[18] << 8) | buf[19];
  *h = ((uint32_t)buf[20] << 24) | ((uint32_t)buf[21] << 16) | ((uint32_t)buf[22] << 8) | buf[23];
  return (*w > 0 && *h > 0);
}

// Finds the raw PCM data inside a WAV buffer and returns a pointer to it.
// Rejects anything that isn't uncompressed PCM (format code 1) — other formats
// would make the duration math wrong.
static bool parseWavHeader(const uint8_t* buf, size_t sz,
                            const uint8_t** pcmOut, size_t* pcmLen,
                            uint32_t* sampleRate, uint8_t* channels, uint8_t* bitsPerSample) {
  if (sz < 44) return false;
  if (memcmp(buf, "RIFF", 4) != 0 || memcmp(buf + 8, "WAVE", 4) != 0) return false;
  uint16_t audioFmt = (uint16_t)buf[20] | ((uint16_t)buf[21] << 8);
  if (audioFmt != 1) return false;
  *channels      = buf[22];
  *sampleRate    = (uint32_t)buf[24] | ((uint32_t)buf[25] << 8) |
                   ((uint32_t)buf[26] << 16) | ((uint32_t)buf[27] << 24);
  *bitsPerSample = buf[34];
  uint32_t pos = 12;
  while (pos + 8 <= (uint32_t)sz) {
    uint32_t chunkSize = (uint32_t)buf[pos+4] | ((uint32_t)buf[pos+5] << 8) |
                         ((uint32_t)buf[pos+6] << 16) | ((uint32_t)buf[pos+7] << 24);
    if (memcmp(buf + pos, "data", 4) == 0) {
      *pcmOut = buf + pos + 8;
      *pcmLen = (pos + 8 + chunkSize <= (uint32_t)sz) ? chunkSize : sz - pos - 8;
      return true;
    }
    if (chunkSize >= (uint32_t)sz) break;  // malformed chunk, would overflow — bail
    pos += 8 + chunkSize;
    if (chunkSize & 1) pos++;  // odd-sized chunks pad to word alignment
  }
  return false;
}

static void playWavBuf(const uint8_t* buf, size_t sz) {
  if (!buf || sz == 0) return;
  const uint8_t* pcm;
  size_t pcmLen;
  uint32_t sr;
  uint8_t ch, bps;
  if (!parseWavHeader(buf, sz, &pcm, &pcmLen, &sr, &ch, &bps)) return;
  bool stereo = (ch > 1);
  if (bps == 16)
    M5.Speaker.playRaw((const int16_t*)pcm, pcmLen / 2, sr, stereo, 1.0f, 0);
  else
    M5.Speaker.playRaw((const int8_t*)pcm, pcmLen, sr, stereo, 1.0f, 0);
}

// Only stop() if something is actually playing. Calling stop() on a silent speaker
// still pokes the AXP2101 and fires a spurious BtnPWR event. But we do need stop()
// when the channel is busy — playRaw busy-waits without yielding, so if the leftover
// audio is longer than the ESP32's task watchdog (~5 s) the device will reset.
void playRevealSound() { if (M5.Speaker.isPlaying()) M5.Speaker.stop(); playWavBuf(wavReveal, wavRevealSz); }
void playClearSound()  { if (M5.Speaker.isPlaying()) M5.Speaker.stop(); playWavBuf(wavClear,  wavClearSz);  }


char randChar() {
  return ALPHABET[random(ALPHA_LEN)];
}

int batteryPercent() {
  float v = (float)M5.Power.getBatteryVoltage();
  if (v > 10.0f) v /= 1000.0f;
  if (v >= 4.20f) return 100;
  if (v <= 3.30f) return 0;

  const float V[] = { 4.20f, 4.10f, 4.00f, 3.90f, 3.80f, 3.70f, 3.60f, 3.50f, 3.30f };
  const int   P[] = { 100,   90,    80,    65,    45,    25,    10,     3,     0    };
  for (int i = 0; i < 8; i++) {
    if (v <= V[i] && v >= V[i+1]) {
      float t = (v - V[i+1]) / (V[i] - V[i+1]);
      return constrain((int)lroundf(P[i+1] + t * (P[i] - P[i+1])), 0, 100);
    }
  }
  return 0;
}


int fitSize(const char* s, int maxSz, int margin) {
  int wMax = M5.Display.width()  - 2 * margin;
  int hMax = M5.Display.height() - 2 * margin;
  canvas.setTextFont(1);
  int best = 1;
  for (int sz = 1; sz <= maxSz; sz++) {
    canvas.setTextSize(sz);
    if (canvas.textWidth(s) <= wMax && canvas.fontHeight() <= hMax) best = sz;
    else break;
  }
  return best;
}

int fitSizeLines(const char* const* lines, int n, int maxSz, int margin) {
  int wMax = M5.Display.width()  - 2 * margin;
  int hMax = M5.Display.height() - 2 * margin;
  canvas.setTextFont(1);
  int best = 1;
  for (int sz = 1; sz <= maxSz; sz++) {
    canvas.setTextSize(sz);
    int maxW = 0;
    for (int i = 0; i < n; i++) {
      int w = canvas.textWidth(lines[i]);
      if (w > maxW) maxW = w;
    }
    int totalH = n * canvas.fontHeight() + (n - 1) * LINE_GAP;
    if (maxW <= wMax && totalH <= hMax) best = sz;
    else break;
  }
  return best;
}


void drawSingle(const char* s, uint32_t color) {
  canvas.fillScreen(COL_BG);
  canvas.setTextFont(1);
  canvas.setTextColor(color);
  canvas.setTextDatum(middle_center);
  canvas.setTextSize(fitSize(s, 10, 8));
  canvas.drawString(s, M5.Display.width() / 2, M5.Display.height() / 2);
  canvas.pushSprite(0, 0);
}

void drawBootScreen() {
  const char* text = "- Click to Receive -";
  uint32_t glowCol = canvas.color565(66, 105, 184);
  int w = M5.Display.width();
  int h = M5.Display.height();

  canvas.fillScreen(COL_BG);

  canvas.setTextFont(1);
  int sz = fitSize(text, 6, 4);
  canvas.setTextSize(sz);
  int textH = canvas.fontHeight();

  const int GAP = 3;

  int imgSize = h - textH - GAP - 4;
  if (imgSize > w) imgSize = w;

  int totalH  = imgSize + GAP + textH;
  int startY  = (h - totalH) / 2;
  int imgX    = (w - imgSize) / 2;
  int imgY    = startY;
  int textCY  = startY + imgSize + GAP + textH / 2;

  if (bootImgBuf && bootImgSz > 0) {
    uint32_t pngW = imgSize, pngH = imgSize;
    getPngSize(bootImgBuf, bootImgSz, &pngW, &pngH);
    // drawPng won't scale for us, so we do it manually
    float scale = (float)imgSize / (float)(pngW > pngH ? pngW : pngH);
    int scaledW = (int)(pngW * scale);
    int scaledH = (int)(pngH * scale);
    int drawX = imgX + (imgSize - scaledW) / 2;
    int drawY = imgY + (imgSize - scaledH) / 2;
    canvas.drawPng(bootImgBuf, bootImgSz, drawX, drawY, 0, 0, 0, 0, scale, scale);
  }

  canvas.setTextDatum(middle_center);

  // draw the same text slightly offset in blue first, white on top — fakes a glow
  canvas.setTextColor(glowCol);
  for (int dx = -2; dx <= 2; dx++)
    for (int dy = -2; dy <= 2; dy++)
      if (dx != 0 || dy != 0)
        canvas.drawString(text, w / 2 + dx, textCY + dy);

  canvas.setTextColor(TFT_WHITE);
  canvas.drawString(text, w / 2, textCY);
  canvas.pushSprite(0, 0);
}

void drawMultiLine(const char* const* lines, int n, uint32_t color, int maxSz, int margin) {
  canvas.fillScreen(COL_BG);
  canvas.setTextFont(1);
  canvas.setTextColor(color);
  canvas.setTextDatum(top_center);
  canvas.setTextSize(fitSizeLines(lines, n, maxSz, margin));
  int fh = canvas.fontHeight();
  int y  = (M5.Display.height() - (n * fh + (n - 1) * LINE_GAP)) / 2;
  for (int i = 0; i < n; i++)
    canvas.drawString(lines[i], M5.Display.width() / 2, y + i * (fh + LINE_GAP));
  canvas.pushSprite(0, 0);
}

void drawPrescript(const char* const* lines, int n, uint32_t color) {
  drawMultiLine(lines, n, color, 10, 4);
}

void drawStatus(const char* const* lines, int n, uint32_t color) {
  drawMultiLine(lines, n, color, 18, 0);
}


void animateSingle(const char* finalStr, uint32_t color,
                   uint32_t spin_ms = SPIN_MS, uint32_t reveal_ms = REVEAL_MS) {
  if (reveal_ms == 0) reveal_ms = 1;
  uint32_t elapsed = millis() - stateStart;
  int len = (int)strlen(finalStr);
  if (len > 63) len = 63;
  char buf[64];

  if (elapsed < spin_ms) {
    for (int i = 0; i < len; i++) buf[i] = randChar();
  } else {
    uint32_t prog = min(elapsed - spin_ms, reveal_ms);
    // uint64_t keeps this from overflowing on long strings
    int revealed  = constrain((int)((uint64_t)prog * len / reveal_ms), 0, len);
    for (int i = 0; i < len; i++)
      buf[i] = (i < revealed) ? finalStr[i] : randChar();
  }
  buf[len] = '\0';
  drawSingle(buf, color);
}

void animateMultiLine(const char* const* finalLines, int n, uint32_t color,
                      void (*drawFn)(const char* const*, int, uint32_t), int cap,
                      uint32_t spin_ms = SPIN_MS, uint32_t reveal_ms = REVEAL_MS) {
  uint32_t elapsed = millis() - stateStart;
  int rows = min(n, cap);

  static char anim[MAX_LINES][64];
  const char* ptrs[MAX_LINES];
  for (int i = 0; i < rows; i++) ptrs[i] = anim[i];

  uint32_t rev_ms = (reveal_ms > 0) ? reveal_ms : 1;
  uint32_t prog = (elapsed >= spin_ms) ? min(elapsed - spin_ms, rev_ms) : 0;

  for (int r = 0; r < rows; r++) {
    int len = (int)strlen(finalLines[r]);
    if (len > 63) len = 63;
    // uint64_t keeps this from overflowing on long strings
    int revealed = (elapsed < spin_ms)
      ? 0
      : constrain((int)((uint64_t)prog * len / rev_ms), 0, len);
    for (int c = 0; c < len; c++)
      anim[r][c] = (c < revealed) ? finalLines[r][c] : randChar();
    anim[r][len] = '\0';
  }
  drawFn(ptrs, rows, color);
}

void animatePrescript(const char* const* lines, int n, uint32_t color) {
  animateMultiLine(lines, n, color, drawPrescript, MAX_LINES);
}

void animateStatus(const char* const* lines, int n, uint32_t color) {
  animateMultiLine(lines, n, color, drawStatus, 4, MSG_SPIN_MS, MSG_REVEAL_MS);
}

void powerOff() {
  prefs.putULong("totalClear", totalClear);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setBrightness(0);
  M5.Display.sleep();
  digitalWrite(PIN_HOLD, LOW);
  while (true) delay(1000);
}


void wrapIntoLines(const char* text) {
  lineCount = 0;
  int len = strlen(text);
  int pos = 0;

  // aim for ~7 lines so the font size stays consistent regardless of text length
  int wrapAt = constrain(len / 7, 15, 35);

  while (pos < len && lineCount < MAX_LINES) {
    while (pos < len && text[pos] == ' ') pos++;
    if (pos >= len) break;

    if (len - pos <= wrapAt) {
      strncpy(prescriptLines[lineCount], text + pos, 63);
      prescriptLines[lineCount++][63] = '\0';
      break;
    }

    int cut = pos + wrapAt;
    for (int i = cut; i > pos; i--) {
      if (text[i] == ' ' && (i - pos) >= 4) { cut = i; break; }
    }

    int chunk = constrain(cut - pos, 0, 63);
    strncpy(prescriptLines[lineCount], text + pos, chunk);
    prescriptLines[lineCount++][chunk] = '\0';
    pos = cut;
  }

  for (int i = 0; i < lineCount; i++)
    prescriptPtrs[i] = prescriptLines[i];
}

void generatePrescript() {
  char buf[512] = {};

  switch (random(16)) {

    case 0:
      strncpy(buf, singles[random(COUNT(singles))], 511);
      break;

    case 1:
      snprintf(buf, 512, "%s%s%s.",
        times[random(COUNT(times))],
        actions[random(COUNT(actions))],
        targets[random(COUNT(targets))]);
      break;

    case 2:
      snprintf(buf, 512, "%s%s%s.%s",
        times[random(COUNT(times))],
        actions[random(COUNT(actions))],
        targets[random(COUNT(targets))],
        postscripts[random(COUNT(postscripts))]);
      break;

    case 3: {
      const char* act = actions[random(COUNT(actions))];
      char cap[64]; strncpy(cap, act, 63); cap[63] = '\0';
      cap[0] = toupper((unsigned char)cap[0]);
      snprintf(buf, 512, "%s%s.%s",
        cap,
        targets[random(COUNT(targets))],
        postscripts[random(COUNT(postscripts))]);
      break;
    }

    case 4:
      snprintf(buf, 512, "%s%s%s. %s %s%s.",
        times[random(COUNT(times))],
        actions[random(COUNT(actions))],
        targets[random(COUNT(targets))],
        transitions[random(COUNT(transitions))],
        actions[random(COUNT(actions))],
        targets[random(COUNT(targets))]);
      break;

    case 5: {
      const char* act = actions[random(COUNT(actions))];
      char cap[64]; strncpy(cap, act, 63); cap[63] = '\0';
      cap[0] = toupper((unsigned char)cap[0]);
      snprintf(buf, 512, "Go to %s. %s%s.%s",
        ny_locA[random(COUNT(ny_locA))],
        cap,
        targets[random(COUNT(targets))],
        postscripts[random(COUNT(postscripts))]);
      break;
    }

    case 6:
      snprintf(buf, 512, "Find the %s %s in %s. %s them.",
        ny_pId2[random(COUNT(ny_pId2))],
        ny_pType[random(COUNT(ny_pType))],
        ny_locA[random(COUNT(ny_locA))],
        ny_v2[random(COUNT(ny_v2))]);
      break;

    case 7:
      snprintf(buf, 512, "%slook for a %s %s in %s.",
        times[random(COUNT(times))],
        ny_pId[random(COUNT(ny_pId))],
        ny_pType[random(COUNT(ny_pType))],
        ny_locA[random(COUNT(ny_locA))]);
      break;

    case 8:
      snprintf(buf, 512, "Find a %s %s wearing a %s %s at %s. %s",
        ny_pId[random(COUNT(ny_pId))],
        ny_pType[random(COUNT(ny_pType))],
        ny_mat[random(COUNT(ny_mat))],
        ny_cloth[random(COUNT(ny_cloth))],
        ny_locA[random(COUNT(ny_locA))],
        nc_act2[random(COUNT(nc_act2))]);
      break;

    case 9: {
      const char* act = actions[random(COUNT(actions))];
      char cap[64]; strncpy(cap, act, 63); cap[63] = '\0';
      cap[0] = toupper((unsigned char)cap[0]);
      snprintf(buf, 512, "Bring %s. %s%s.%s",
        ny_objcs[random(COUNT(ny_objcs))],
        cap,
        targets[random(COUNT(targets))],
        postscripts[random(COUNT(postscripts))]);
      break;
    }

    case 10:
      snprintf(buf, 512, "Play %s with %s.%s",
        ny_games[random(COUNT(ny_games))],
        targets[random(COUNT(targets))],
        postscripts[random(COUNT(postscripts))]);
      break;

    case 11:
      snprintf(buf, 512, "%sspeak about %s with %s.%s",
        times[random(COUNT(times))],
        ny_topic[random(COUNT(ny_topic))],
        targets[random(COUNT(targets))],
        postscripts[random(COUNT(postscripts))]);
      break;

    case 12:
      snprintf(buf, 512, "%s%s",
        times[random(COUNT(times))],
        nc_act2[random(COUNT(nc_act2))]);
      break;

    case 13:
      snprintf(buf, 512, "%s%s%s",
        nc_act1[random(COUNT(nc_act1))],
        nc_markers[random(COUNT(nc_markers))],
        nc_act2[random(COUNT(nc_act2))]);
      break;

    case 14:
      snprintf(buf, 512, "%s%s%s",
        times[random(COUNT(times))],
        nc_act2[random(COUNT(nc_act2))],
        postscripts[random(COUNT(postscripts))]);
      break;

    default:
      snprintf(buf, 512, "%s%s%s%s",
        nc_act1[random(COUNT(nc_act1))],
        nc_markers[random(COUNT(nc_markers))],
        nc_act2[random(COUNT(nc_act2))],
        postscripts[random(COUNT(postscripts))]);
      break;
  }

  buf[511] = '\0';
  wrapIntoLines(buf);

  if (lineCount == 0) {
    strncpy(prescriptLines[0], "...", 63);
    prescriptLines[0][63] = '\0';
    prescriptPtrs[0] = prescriptLines[0];
    lineCount = 1;
  }
}


void enterMode(Mode m) {
  mode       = m;
  stateStart = millis();
  lastFrame  = 0;
  holding    = false;
}

void nextPrescript() {
  generatePrescript();
  // enterMode first — stateStart has to be set before the sound plays, or frame 0 gets a wrong timestamp
  enterMode(Mode::ANIMATING);
  playRevealSound();
}

void showMessage(const char* msg) {
  strncpy(target, msg, 63);
  target[63] = '\0';
  enterMode(Mode::MESSAGE);
}

void showStatus() {
  playRevealSound();
  snprintf(statusLines[0], 64, "_Welcome back,");
  snprintf(statusLines[1], 64, "%s._", USER_NAME);
  snprintf(statusLines[2], 64, "_Battery._ %d%%", batteryPercent());
  snprintf(statusLines[3], 64, "_TOTAL_CLEAR._ %lu", (unsigned long)totalClear);
  for (int i = 0; i < 4; i++) statusPtrs[i] = statusLines[i];
  enterMode(Mode::STATUS);
}

void clearPrescript() {
  totalClear++;
  playClearSound();
  strncpy(target, "_CLEAR._", 63); target[63] = '\0';
  enterMode(Mode::CLEARING);
}


void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  pinMode(PIN_HOLD, OUTPUT);
  digitalWrite(PIN_HOLD, HIGH);

  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  btStop();
  setCpuFrequencyMhz(80);

  M5.Display.setRotation(1);
  M5.Display.setBrightness(BRIGHTNESS);
  M5.Speaker.setVolume(128);

  canvas.createSprite(M5.Display.width(), M5.Display.height());

  prefs.begin("slotapp", false);
  totalClear = prefs.getULong("totalClear", 0);

  LittleFS.begin(true);
  loadWav("/index_message_1.wav", &wavReveal,  &wavRevealSz);
  loadWav("/index_message_2.wav", &wavClear,   &wavClearSz);
  loadWav("/index_new.png",       &bootImgBuf, &bootImgSz);

  // sync the animation length to the actual sound duration
  {
    const uint8_t* pcm; size_t pcmLen; uint32_t sr; uint8_t ch, bps;
    if (wavReveal && parseWavHeader(wavReveal, wavRevealSz, &pcm, &pcmLen, &sr, &ch, &bps)
        && sr > 0 && ch > 0 && bps >= 8) {
      uint32_t soundMs = (uint32_t)((uint64_t)pcmLen * 1000 /
                         ((uint64_t)sr * ch * (bps / 8)));
      if (soundMs > SPIN_MS && soundMs <= 10000) {
        REVEAL_MS = soundMs - SPIN_MS;
        ANIM_MS   = soundMs;
      }
    }
  }

  randomSeed((uint32_t)esp_random());
  M5.update();  // BtnA is the physical power button — its press from boot sits in the event buffer, clear it
  lastActivity = millis();
  btnLockout   = millis() + 1500;
  enterMode(Mode::BOOT);
}

void loop() {
  M5.update();
  // tiny yield so millis() is fresh
  delay(1);
  uint32_t now = millis();

  // BtnPWR: hold 3 s to shut down. On short release, restore the display —
  // the AXP2101 cuts the backlight on any power-key event, leaving the screen black.
  if (M5.BtnPWR.isPressed()) {
    if (btnPWRStart == 0) btnPWRStart = now;
    if (mode != Mode::SLEEPING && now - btnPWRStart >= 3000) {
      strncpy(target, SLEEP_TEXT, 63); target[63] = '\0';
      enterMode(Mode::SLEEPING);
      btnPWRStart = 0;
    }
  } else if (btnPWRStart != 0) {
    M5.Display.wakeup();
    M5.Display.setBrightness(BRIGHTNESS);
    btnPWRStart = 0;
  }

  if (mode != Mode::SLEEPING) {
    if (now - lastActivity >= INACTIVITY_MS) {
      strncpy(target, SLEEP_TEXT, 63); target[63] = '\0';
      enterMode(Mode::SLEEPING);
    }

    if (now >= btnLockout && M5.BtnA.wasPressed()) {
      btnLockout    = now + DEBOUNCE;
      lastActivity  = now;

      if (mode == Mode::BOOT) {
        nextPrescript();
        return;
      }
      if (mode == Mode::STATUS || mode == Mode::STATUS_HOLD) {
        enterMode(Mode::BOOT);
        return;
      }
      if (mode == Mode::ANIMATING || mode == Mode::CLEARING ||
          mode == Mode::CLEARING_HOLD || mode == Mode::MESSAGE) {
        return;
      }
      if (mode == Mode::READING) {
        clearPrescript();
        return;
      }
    }

    if (now >= btnLockout && M5.BtnB.wasPressed()) {
      btnLockout    = now + DEBOUNCE;
      lastActivity  = now;
      if (mode == Mode::ANIMATING || mode == Mode::CLEARING ||
          mode == Mode::CLEARING_HOLD || mode == Mode::MESSAGE) { return; }
      if (mode == Mode::STATUS || mode == Mode::STATUS_HOLD) { return; }
      showStatus();
      return;
    }
  }

  switch (mode) {

    case Mode::BOOT: {
      if (!holding) { drawBootScreen(); holding = true; }
      break;
    }

    case Mode::ANIMATING: {
      if (now - stateStart >= ANIM_MS) {
        enterMode(Mode::READING);
        drawPrescript(prescriptPtrs, lineCount, COL_TEXT);
      } else if (now - lastFrame >= FRAME_MS) {
        lastFrame = now;
        animatePrescript(prescriptPtrs, lineCount, COL_TEXT);
      }
      break;
    }

    case Mode::READING: {
      break;
    }

    case Mode::CLEARING: {
      if (now - stateStart >= MSG_ANIM_MS) { enterMode(Mode::CLEARING_HOLD); }
      else if (now - lastFrame >= FRAME_MS) { lastFrame = now; animateSingle(target, COL_TEXT, MSG_SPIN_MS, MSG_REVEAL_MS); }
      break;
    }

    case Mode::CLEARING_HOLD: {
      if (!holding) { drawSingle(target, COL_TEXT); holding = true; }
      if (now - stateStart >= MSG_HOLD) { enterMode(Mode::BOOT); }
      break;
    }

    case Mode::MESSAGE: {
      uint32_t elapsed = now - stateStart;
      uint32_t color   = (strcmp(target, "_ERROR._") == 0) ? TFT_RED : COL_TEXT;
      if (elapsed < MSG_ANIM_MS) {
        if (now - lastFrame >= FRAME_MS) { lastFrame = now; animateSingle(target, color, MSG_SPIN_MS, MSG_REVEAL_MS); }
      } else if (elapsed < MSG_ANIM_MS + MSG_HOLD) {
        if (!holding) { drawSingle(target, color); holding = true; }
      } else {
        enterMode(Mode::BOOT);
      }
      break;
    }

    case Mode::STATUS: {
      if (now - stateStart >= MSG_ANIM_MS) { enterMode(Mode::STATUS_HOLD); }
      else if (now - lastFrame >= FRAME_MS) { lastFrame = now; animateStatus(statusPtrs, 4, COL_TEXT); }
      break;
    }

    case Mode::STATUS_HOLD: {
      if (!holding) { drawStatus(statusPtrs, 4, COL_TEXT); holding = true; }
      if (now - stateStart >= STATUS_HOLD_MS) { enterMode(Mode::BOOT); }
      break;
    }

    case Mode::SLEEPING: {
      uint32_t elapsed = now - stateStart;

      if (elapsed < ANIM_MS) {
        if (now - lastFrame >= FRAME_MS) { lastFrame = now; animateSingle(target, COL_TEXT); }
      } else if (elapsed < ANIM_MS + SLEEP_DUR) {
        if (!holding) { drawSingle(target, COL_TEXT); holding = true; }
      } else {
        powerOff();
      }
      break;
    }
  }
}
