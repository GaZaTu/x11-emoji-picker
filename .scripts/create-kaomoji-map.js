const { promises: { writeFile, readdir }, existsSync } = require('fs')
const { execFile } = require('child_process')
const { default: fetch } = require('node-fetch')
const ProxyAgent = require('proxy-agent')
const html = require('node-html-parser')

const KAOMOJI_LIST_URL = `https://slangit.com/emoticons/kaomoji`

const HTTP_PROXY = process.env.HTTP_PROXY
const httpProxyAgent = HTTP_PROXY && new ProxyAgent(HTTP_PROXY)

const fetchKaomojis = async () => {
  const result = []

  const response = await fetch(KAOMOJI_LIST_URL, {
    agent: httpProxyAgent,
  })
  const document = html.parse(await response.text())

  const entries = document.querySelectorAll('table.slist > tbody > tr')
  for (const entry of entries) {
    const children = entry.querySelectorAll('td')

    const text = children[0].text
    const name = children[1].childNodes[0].attributes.href.replace('/emoticon/', '')
    const rating = children[2].querySelector('.rating').classList.toString().replace('rating ', '')
    const validRatings = ['five', 'four-half', 'four']

    if (!validRatings.includes(rating)) {
      continue
    }

    if (text.includes('\n')) {
      continue
    }

    result.push({
      name,
      text,
    })
  }

  return result
}

void (async () => {
  const kaomojisHpp =
  `
#pragma once

#include <string>

struct Kaomoji {
public:
  std::string name;
  std::string text;

  bool operator==(const Kaomoji& other) const {
    return name == other.name;
  }

  operator bool() const {
    return text != "";
  }
};

// generated from ${KAOMOJI_LIST_URL}
const Kaomoji kaomojis[] = {
  ${(await fetchKaomojis()).map(emoji => `{"${emoji.name}", u8"${emoji.text.replaceAll('\\', '\\\\').replaceAll('"', '\\"')}"}`).join(',\n  ')}
};
  `

  await writeFile(`${__dirname}/../src/kaomojis.hpp`, kaomojisHpp.trimLeft())
})()
