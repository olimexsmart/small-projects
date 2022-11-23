// Global vars
const givenAnswers = []
const correctAnswers = []
// Startup
document.addEventListener('DOMContentLoaded', () => {
  const quizContainerDiv = document.getElementById('quizMainContainer')
  const quizTitleSpan = document.getElementById('quizTitleSpan')
  const timeLeftSpan = document.getElementById('timeLeftSpan')
  const timeExpiredModal = new bootstrap.Modal(document.getElementById('timeExpiredModal'))
  const examResultModal = new bootstrap.Modal(document.getElementById('examResultModal'))
  const resultTitleModal = document.getElementById('resultTitleModal')
  const resultTextModal = document.getElementById('resultTextModal')
  const resultHeaderModal = document.getElementById('resultHeaderModal')
  // const figureBodyModal = document.getElementById('figureBodyModal')

  // Timer reference from when the page was loaded
  let endTime = new Date()
  endTime.setMinutes(endTime.getMinutes() + 45)
  endTime = endTime.getTime()
  const minCorrect = 36
  // Answer check
  document.getElementById('checkAnswersBtn').addEventListener('click', () => {
    let nCorrect = 0
    let nEmpty = 0
    for (let n = 0; n < correctAnswers.length; n++) {
      const correctAnswerEl = document.getElementById(`answ-${n}-${correctAnswers[n]}`)
      correctAnswerEl.classList.remove('givenAnswer')
      if (correctAnswers[n] === givenAnswers[n]) { // Correct answer
        correctAnswerEl.classList.add('correctAnswer')
        nCorrect++
      } else { // Wrong answer
        correctAnswerEl.classList.add('wrongAnswer')
        if (givenAnswers[n] === undefined) {
          nEmpty++
        }
      }
    }
    // Prepare exam result modal
    if (nCorrect >= minCorrect) { // Exam passed
      resultTitleModal.textContent = 'Idoneo'
      resultHeaderModal.classList.add('correctAnswer')
      resultHeaderModal.classList.remove('wrongAnswer')
    } else { // Exam Failed
      resultTitleModal.textContent = 'Non Idoneo'
      resultHeaderModal.classList.remove('correctAnswer')
      resultHeaderModal.classList.add('wrongAnswer')
    }
    resultTextModal.innerText = `Risposte corrette necessarie: ${minCorrect}\n` +
      `Risposte corrette: ${nCorrect} su ${correctAnswers.length}\n` +
      `Non compilate: ${nEmpty}`
    examResultModal.show()
  })
  // Reload page to get new quiz
  document.getElementById('newQuizBtn').addEventListener('click', () => {
    location.reload();
  })
  // Request quiz
  const apiPath = `quiz.php`
  fetch(apiPath).then((response) => {
    if (response.ok) {
      response.json().then(json => {
        // Pupulate HTML with answers
        for (let i = 0; i < json.length; i++) {
          appendQuizEntry(quizContainerDiv, json[i], i)
        }
      })
    } else {
      console.log(response.status)
    }
  })
  // Start quiz timer
  let timerHandle = setInterval(() => {
    const delta = endTime - new Date().getTime()
    if (delta <= 0) {
      clearInterval(timerHandle)
      timeExpiredModal.show()
      timeLeftSpan.textContent = `00:00`
    } else {
      const minutes = Math.floor((delta % (1000 * 60 * 60)) / (1000 * 60))
      const seconds = Math.floor((delta % (1000 * 60)) / 1000)
      timeLeftSpan.textContent = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0')
    }
  })
})

// Appends one quiz question to the page
function appendQuizEntry(parentEl, quizEntry, n) {
  // Bootstrap containers
  const row = document.createElement('div')
  row.classList.add('row')
  const col = document.createElement('div')
  col.classList.add('col')
  // Quiz complete container
  const quizCard = document.createElement('div')
  quizCard.classList.add('border', 'mt-2', 'quizCard', 'darkRedBorder')
  // Card header with question text
  const quizCardHeader = document.createElement('div')
  quizCardHeader.classList.add('d-flex', 'flex-row', 'border-bottom', 'text-start', 'quizHeader', 'darkRedBorder')
  // Div to contain SVG
  const noNameDiv1 = document.createElement('div')
  noNameDiv1.innerHTML = `
  <svg width="50" height="50">
    <polygon points="0,0 0,50 50,0" style="fill:darkred;" />
    <text x="10" y="20" fill="#fbe9e9">${n + 1}</text>
  </svg>
  `
  // Div to contain question text
  const noNameDiv2 = document.createElement('div')
  noNameDiv2.textContent = quizEntry.question
  // Answers section
  const quizCardAnswers = document.createElement('div')
  quizCardAnswers.classList.add('d-flex', 'flex-column')
  for (let ia = 0; ia < quizEntry.answers.length; ia++) {
    const answer = document.createElement('div')
    answer.id = `answ-${n}-${ia}`
    answer.setAttribute('questN', n)
    answer.setAttribute('answN', ia)
    // CSS depends on the answer position
    if (ia != 1) {
      answer.classList.add('p-2', 'quizAnswer', 'cursorPointer')
    } else {
      answer.classList.add('p-2', 'quizAnswer', 'cursorPointer', 'border-bottom', 'border-top', 'darkRedBorder')
    }
    answer.textContent = quizEntry.answers[ia].answer
    answer.onclick = answerGiven
    quizCardAnswers.appendChild(answer)
    // Save correct answer on global array
    if (quizEntry.answers[ia].correct === '1') {
      correctAnswers[n] = ia
    }
  }
  // Combine all elements together
  quizCardHeader.appendChild(noNameDiv1)
  quizCardHeader.appendChild(noNameDiv2)
  quizCard.appendChild(quizCardHeader)
  quizCard.appendChild(quizCardAnswers)
  col.appendChild(quizCard)
  row.appendChild(col)
  parentEl.appendChild(row)
}

// Callback on answer click
function answerGiven(ev) {
  const el = ev.target
  givenAnswers[parseInt(el.getAttribute('questN'))] = parseInt(el.getAttribute('answN'))
  // Remove selection mark from all answers
  for (const answerDiv of el.parentElement.childNodes) {
    answerDiv.classList.remove('givenAnswer', 'correctAnswer', 'wrongAnswer')
  }
  // Mark as selected
  el.classList.add('givenAnswer')
}

// Hide figure modal
// function hideFigureModal() {
//   figureBodyModal.hide()
// }
