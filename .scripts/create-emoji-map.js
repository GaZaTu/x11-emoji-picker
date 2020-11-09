const { promises: { writeFile }, existsSync } = require('fs')
const { default: fetch } = require('node-fetch')
const ProxyAgent = require('proxy-agent')

const UNICODE_EMOJI_LIST_URL = 'https://unicode.org/Public/emoji/13.0/emoji-test.txt'
const UNICODE_EMOJI_ANNOTATIONS_BASE_URL = 'https://raw.githubusercontent.com/unicode-org/cldr/release-38/common/annotations'

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
]

/**
 * @returns {Promise<{ name: string, code: string, str: string, version: number }[]>}
 */
const fetchEmojis = async () => {
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
 * @returns {Promise<{ name: string, code: string, str: string }[]>}
 */
const fetchEmojisByLocale = async (locale) => {
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

/**
 * @param {string[]} locales
 * @returns {Promise<{ locale: string, emojis: { name: string, code: string, str: string }[] }[]>}
 */
const fetchEmojisByLocales = async (locales) => {
  const emojis = []

  for (const locale of locales) {
    emojis.push({ locale, emojis: await fetchEmojisByLocale(locale) })
  }

  return emojis
}

(async () => {
  const NO_CLDR = process.argv.includes('NO_CLDR')
  const SKIP_HPP = process.argv.includes('SKIP_HPP')
  const SKIP_CPP = process.argv.includes('SKIP_CPP')

  if (!SKIP_HPP) {
    const emojisHpp =
`
#pragma once

#include <locale>
#include <string>
#include <vector>

struct Emoji {
public:
  std::string name;
  std::string code;
  short version = 0;

  ${NO_CLDR ? '// NO_CLDR' : 'std::string nameByLocale(const std::string& localeKey = std::locale("").name().substr(0, 2)) const;'}

  bool operator==(const Emoji& other) const {
    return code == other.code;
  }
};

// generated from ${UNICODE_EMOJI_LIST_URL}
const std::vector<Emoji> emojis = {
  ${(await fetchEmojis()).map(emoji => `{"${emoji.name}", u8"${emoji.code.split(' ').map(codepoint => `\\U${codepoint.padStart(8, '0')}`).join('')}", ${Math.trunc(emoji.version)}}`).join(',\n  ')}
};
`

    await writeFile(`${__dirname}/../src/emojis.hpp`, emojisHpp.trimLeft())
  }

  if (!SKIP_CPP && !NO_CLDR) {
    const emojisCpp =
`
#include "emojis.hpp"
#include <unordered_map>

const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> locales = {
  ${(await fetchEmojisByLocales(supportedLocales)).map(({ locale, emojis }) => {
    return `// generated from ${UNICODE_EMOJI_ANNOTATIONS_BASE_URL}/${locale}.xml
  {"${locale}", {
    ${emojis.map(emoji => `{u8"${emoji.code.split(' ').map(codepoint => `\\U${codepoint.padStart(8, '0')}`).join('')}", u8"${emoji.name}"}`).join(',\n    ')}
  }}`
  }).join(',\n  ')}
};

std::string Emoji::nameByLocale(const std::string& localeKey) const {
  auto foundList = locales.find(localeKey);
  if (foundList == locales.end()) {
    return name;
  }

  auto foundName = foundList->second.find(code);
  if (foundName == foundList->second.end()) {
    return name;
  }

  return foundName->second;
}
`

    await writeFile(`${__dirname}/../src/emojis.cpp`, emojisCpp.trimLeft())
  }
})().catch(console.error)
