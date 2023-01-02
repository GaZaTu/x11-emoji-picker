import { execFile } from 'child_process'
import { existsSync } from 'fs'
import { readdir, writeFile } from 'fs/promises'
import fetch from 'node-fetch'
import ProxyAgent from 'proxy-agent'
import { fileURLToPath } from 'url'

const __dirname = fileURLToPath(new URL('.', import.meta.url))

const UNICODE_EMOJI_LIST_URL = 'https://unicode.org/Public/emoji/14.0/emoji-test.txt'
const UNICODE_EMOJI_ANNOTATIONS_BASE_URL = 'https://raw.githubusercontent.com/unicode-org/cldr/release-42/common/annotations'

const HTTP_PROXY = process.env.HTTP_PROXY
const httpProxyAgent = HTTP_PROXY && new ProxyAgent(HTTP_PROXY)

const supportedLocales = [
  'en',
  'de',
  'fr',
  'nl',
  'da',
  'it',
  'pt',
  'es',
  'sv',
  'pl',
  'hr',
  'cs',
  'fi',
  'el',
  'hu',
  'ru',
]
supportedLocales.sort()

/**
 */
const fetchEmojis = async () => {
  /** @type {{ name: string, code: string, str: string, version: number }[]} */
  const emojis = []

  const emojiTestResponse = await fetch(UNICODE_EMOJI_LIST_URL, { agent: httpProxyAgent })
  const emojiTestRegex = /^(.*);\s*(unqualified|fully-qualified)\s*#\s*(\S*)\s*(E\d+\.\d+|)\s*(.*)$/gm
  const emojiTestTxt = await emojiTestResponse.text()

  let match
  while ((match = emojiTestRegex.exec(emojiTestTxt)) !== null) {
    if (match.index === emojiTestRegex.lastIndex) {
      emojiTestRegex.lastIndex++
    }

    const codepoints = [...match[3]].map(cp => cp.codePointAt(0).toString(16))
    const filename = `${__dirname}/../src/res/72x72/${codepoints.join('-')}.png`

    if (existsSync(filename)) {
      emojis.push({
        name: match[5].toLowerCase().replace(/['.:,()“”]/g, '').replace(/[ \-]/g, '_').replace(/(___|__)/g, '_'),
        code: match[1].trim(),
        str: match[3],
        version: parseFloat((match[4] || 'E0.0').slice(1)),
      })
    }
  }

  return emojis
}

/**
 * @param {string} locale
 */
const fetchEmojisByLocale = async (locale) => {
  /** @type {{ name: string, code: string, str: string }[]} */
  const emojis = []

  const UNICODE_EMOJI_LOCALE_LIST_URL = `${UNICODE_EMOJI_ANNOTATIONS_BASE_URL}/${locale}.xml`

  const emojiLocaleResponse = await fetch(UNICODE_EMOJI_LOCALE_LIST_URL, { agent: httpProxyAgent })
  const emojiLocaleRegex = /<annotation cp="([^"]+)" type="tts">([^<]+)<\/annotation>/gm
  const emojiLocaleTxt = await emojiLocaleResponse.text()

  let match
  while ((match = emojiLocaleRegex.exec(emojiLocaleTxt)) !== null) {
    if (match.index === emojiLocaleRegex.lastIndex) {
      emojiLocaleRegex.lastIndex++
    }

    const codepoints = [...match[1]].map(cp => cp.codePointAt(0).toString(16))
    const filename = `${__dirname}/../src/res/72x72/${codepoints.join('-')}.png`

    if (existsSync(filename)) {
      emojis.push({
        name: match[2],
        code: codepoints.map(cp => cp.toUpperCase()).join(' '),
        str: match[1],
      })
    }
  }

  return emojis
}

const NO_CLDR = process.argv.includes('NO_CLDR')
const SKIP_HPP = process.argv.includes('SKIP_HPP')
const SKIP_CPP = process.argv.includes('SKIP_CPP')
const USE_GPERF = process.argv.includes('USE_GPERF')
const SKIP_QRC = process.argv.includes('SKIP_QRC')

const emojis = await fetchEmojis()

if (!SKIP_HPP) {
  const emojisHpp =
`
#pragma once

#include <locale>
#include <string>

struct Emoji {
public:
  std::string name;
  std::string code;
  short version = -1;

  std::string nameByLocale(const std::string& localeKey = std::locale("").name().substr(0, 2)) const;

  bool isGenderVariation() const;

  bool isSkinToneVariation() const;

  bool operator==(const Emoji& other) const;

  operator bool() const;

private:
  template <int LOCALE_ID>
  std::string nameByLocaleId() const;

  static constexpr unsigned int hashLocaleKey(const char* s, int off = 0) {
    return !s[off] ? 5381 : (hashLocaleKey(s, off + 1) * 33) ^ s[off];
  }
};

// generated from ${UNICODE_EMOJI_LIST_URL}
extern const Emoji emojis[${emojis.length}];
`

  await writeFile(`${__dirname}/../src/emojis.hpp`, emojisHpp.trimStart())
}

if (!SKIP_CPP) {
  const emojisCpp =
`
#include "emojis.hpp"

std::string Emoji::nameByLocale(const std::string& localeKey) const {
  ${NO_CLDR ? '// NO CLDR' : `
  switch (hashLocaleKey(localeKey.data())) {
    ${supportedLocales.map(localeKey => {
      return `case hashLocaleKey("${localeKey}"): return nameByLocaleId<hashLocaleKey("${localeKey}")>();`
    }).join('\n    ')}
  }`.trimStart()}

  return name;
}

bool Emoji::isGenderVariation() const {
  const char* MALE_SIGN   = u8"\\U00002642";
  const char* FEMALE_SIGN = u8"\\U00002640";
  const char* BOY         = u8"\\U0001F466";
  const char* GIRL        = u8"\\U0001F467";
  const char* MAN         = u8"\\U0001F468";
  const char* WOMAN       = u8"\\U0001F469";

  if (
    code == MALE_SIGN   ||
    code == FEMALE_SIGN ||
    code == BOY         ||
    code == GIRL        ||
    code == MAN         ||
    code == WOMAN
  ) {
    return false;
  }

  return (
    code.find(MALE_SIGN) != std::string::npos   ||
    code.find(FEMALE_SIGN) != std::string::npos ||
    code.find(BOY) != std::string::npos         ||
    code.find(GIRL) != std::string::npos        ||
    code.find(MAN) != std::string::npos         ||
    code.find(WOMAN) != std::string::npos
  );
}

bool Emoji::isSkinToneVariation() const {
  const char* LIGHT        = u8"\\U0001F3FB";
  const char* LIGHT_MEDIUM = u8"\\U0001F3FC";
  const char* MEDIUM       = u8"\\U0001F3FD";
  const char* DARK_MEDIUM  = u8"\\U0001F3FE";
  const char* DARK         = u8"\\U0001F3FF";

  if (
    code == LIGHT        ||
    code == LIGHT_MEDIUM ||
    code == MEDIUM       ||
    code == DARK_MEDIUM  ||
    code == DARK
  ) {
    return false;
  }

  return (
    code.find(LIGHT) != std::string::npos        ||
    code.find(LIGHT_MEDIUM) != std::string::npos ||
    code.find(MEDIUM) != std::string::npos       ||
    code.find(DARK_MEDIUM) != std::string::npos  ||
    code.find(DARK) != std::string::npos
  );
}

bool Emoji::operator==(const Emoji& other) const {
  return code == other.code;
}

Emoji::operator bool() const {
  return code != "";
}

const Emoji emojis[] = {
  ${emojis.map(emoji => `{"${emoji.name}", u8"${emoji.code.split(' ').map(codepoint => `\\U${codepoint.padStart(8, '0')}`).join('')}", ${Math.trunc(emoji.version)}}`).join(',\n  ')}
};
`

  await writeFile(`${__dirname}/../src/emojis.cpp`, emojisCpp.trimStart())

  if (!NO_CLDR) {
    for (const localeKey of supportedLocales) {
      const emojisByLocale = await fetchEmojisByLocale(localeKey)

      if (USE_GPERF) {
        const emojisGPerf =
`
%{
// generated from ${UNICODE_EMOJI_ANNOTATIONS_BASE_URL}/${localeKey}.xml

#include "emojis.hpp"

%}
%language=C++
%readonly-tables
%enum
%compare-lengths
%includes
%struct-type
%define slot-name emoji
%define lookup-function-name find
%define class-name EmojiTranslations_${localeKey}
struct EmojiTranslation_${localeKey} {
  const char* emoji;
  const char* value;
};
%%
${emojisByLocale
  .map(emoji => `${emoji.str}, "${emoji.name}"`)
  .join('\n')}
%%
template <>
std::string Emoji::nameByLocaleId<Emoji::hashLocaleKey("${localeKey}")>() const {
  auto translation = EmojiTranslations_${localeKey}::find(code.data(), code.length());
  if (translation == nullptr) {
    return name;
  }

  return translation->value;
}
`
        const gperf = execFile('gperf', [`--output-file=src/i18n/emojis_${localeKey}.cpp`], {
          cwd: `${__dirname}/..`,
        })

        gperf.stderr.pipe(process.stderr)
        gperf.stdout.pipe(process.stdout)
        gperf.stdin.end(emojisGPerf)

        await new Promise((resolve, reject) => {
          gperf.on('error', reject)
          gperf.on('exit', resolve)
        })
      } else {
        const emojisCpp =
`
// generated from ${UNICODE_EMOJI_ANNOTATIONS_BASE_URL}/${localeKey}.xml

#include "emojis.hpp"
#include <unordered_map>

const std::unordered_map<std::string, std::string> EmojiTranslations_${localeKey} = {
  ${emojisByLocale.map(emoji => `{u8"${emoji.code.split(' ').map(codepoint => `\\U${codepoint.padStart(8, '0')}`).join('')}", u8"${emoji.name}"}`).join(',\n  ')}
};

template <>
std::string Emoji::nameByLocaleId<Emoji::hashLocaleKey("${localeKey}")>() const {
  auto translation = EmojiTranslations_${localeKey}.find(code);
  if (translation == EmojiTranslations_${localeKey}.end()) {
    return name;
  }

  return translation->second;
}
`

        await writeFile(`${__dirname}/../src/i18n/emojis_${localeKey}.cpp`, emojisCpp.trimStart())
      }
    }
  }
}

if (!SKIP_QRC) {
  const emojisQrc =
`
<!DOCTYPE RCC>
<RCC version="1.0">
  <qresource>
    ${(await readdir(`${__dirname}/../src/res/72x72`)).map(f => `<file>res/72x72/${f}</file>`).join('\n    ')}
  </qresource>
</RCC>
`
  await writeFile(`${__dirname}/../src/emojis.qrc`, emojisQrc.trimStart())
}
