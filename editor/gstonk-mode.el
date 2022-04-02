;;; gstonk-mode.el --- Major mode for editing GStonk files -*- lexical-binding: t; -*-
;;
;; Copyright (C) 2022 Nádas András <andrew.reeds.mpg@gmail.com>
;;
;; Author: Andrew Reeds <andrew.reeds.mpg@gmail.com>
;; URL: https://github.com/Andrew-Reeds/GStonk
;; Permission is hereby granted, free of charge, to any person
;; obtaining a copy of this software and associated documentation files (the "Software"),
;; to deal in the Software without restriction, including without limitation
;; the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
;; of the Software, and to permit persons to whom the Software is furnished to do so,
;; subject to the following conditions:
;;
;;The above copyright notice and this permission notice shall be included
;;in all copies or substantial portions of the Software.
;;
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
;; INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
;; IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
;; DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
;; ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;
;; This file is not part of GNU Emacs.
;;
;;; Commentary:
;;  I don't know, what to put here, don't forget to check the TODO-s at the flycheck section.
;;
;;
;;; Code:

(defvar gstonk-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\t" 'gstonk-tabbing)
    (define-key map "`" '(lambda ()
                           "Stopping emacs from inserting two `'s at once."
                           (interactive)
                           (insert-char ?`)))
    map)
  "Keymap for Gstonk mode.")


(defconst gstonk-font-lock
  (list
   '("\"\\(\\.\\)" . font-lock-string-face)
   '("\\$[a-z]+" . font-lock-constant-face)
   '("[#`][0-9]+" . font-lock-variable-name-face)
   '("\\?\\?\\|@@\\|\\?!\\||>\\||\\?\\|<<|\\|\\.\\(if\\|while\\|try\\|else\\|elif\\|catch\\|ret\\)" . font-lock-keyword-face)
   )
  "Highlighting expressions for Gstonk mode.")

(defun gstonk-indent-line ()
  "Indent current line as Gstonk code."
  (interactive)
  (let ((cur-indent 0) (forward (current-column)))
    (save-excursion
      (beginning-of-line)
        (if (not (or (bobp) (looking-at "::\\|<>\\|()\\|\\[")))
            (save-excursion
              (if (looking-at "^[^}\n]*}")
                  (progn
                    (forward-line -1)
                    (while (looking-at "^[[:space:]]*$")
                      (forward-line -1))
                    (if (looking-at "^[^{\n]*{")
                        (setq cur-indent (current-indentation))
                      (setq cur-indent (- (current-indentation) tab-width)))
                    (if (< cur-indent 0)
                        (setq cur-indent 0)))
                (let ((not-indented t))
                  (while not-indented
                    (forward-line -1)
                    (if (looking-at "^[^{\n]*{")
                        (progn
                          (setq cur-indent (+ (current-indentation) tab-width))
                          (setq not-indented nil))
                      (if (looking-at  "^[^}\n]*}")
                          (progn
                            (setq cur-indent (current-indentation))
                            (setq not-indented nil))
                        (if (or (bobp) (looking-at "^\\(::\\|<>\\|()\\|\\[\\)"))
                            (setq not-indented nil))))))))))
    (setq forward (+ forward (- cur-indent (current-indentation))))
    (indent-line-to cur-indent)
    (beginning-of-line)
    (forward-char forward)))

(defun gstonk-align ()
  "Align words in Gstonk code."
  (interactive)
  (if (looking-at "[[:space:]]")
      (while (looking-at "[[:space:]]")
        (forward-char))
    (if indent-tabs-mode
        (insert-char ?\t)
      (let ((alignment 0) (align-to-upper-line nil) (column (current-column)))
        (save-excursion
          (forward-line -1)
          (while (and (looking-at "^[[:space:]]*$") (not (bobp)))
            (forward-line -1))
          (save-excursion
            (beginning-of-line)
            (if (looking-at "^\\(::\\|()\\|<>\\)")
                (progn
                  (end-of-line)
                  (while (looking-back " " 1)
                    (forward-char -1))
                  (setq align-to-upper-line (looking-back ";" 1)))
              (setq align-to-upper-line t))
            (end-of-line)
            (setq align-to-upper-line (and align-to-upper-line (<= column (current-column)))))
          (if align-to-upper-line
              (progn
                (forward-char column)
                (while (not (looking-at "[ \t]\\|$"))
                  (forward-char)
                  (setq alignment (+ alignment 1)))
                (if (looking-at "[[:space:]]*$")
                    (setq align-to-upper-line nil)
                  (while (looking-at " ")
                    (forward-char)
                    (setq alignment (+ alignment 1)))))))
        (if align-to-upper-line
            (insert-char ?\s alignment)
          (insert-char ?\s (- tab-width (% (current-column) tab-width))))))))

(defvar gstonk-last-tab-pos (point-at-bol))
(defun gstonk-tabbing ()
  "Indent or align depending on the context in Gstonk code."
  (interactive)
  (if (looking-at "[ \t]")
      (while (looking-at "[ \t]")
        (forward-char))
    (let ((indent (not (= gstonk-last-tab-pos (point)))))
      (save-excursion
        (while (looking-back "[ \t]" 1)
          (forward-char -1))
        (setq indent (and indent (looking-at "^")))
        (beginning-of-line)
        (setq indent (or indent (looking-at "<>\\|()\\|::\\|\\["))))
      (if indent
          (gstonk-indent-line)
        (gstonk-align))))
  (setq gstonk-last-tab-pos (point)))


(defun gstonk-comment (arg)
  "Create comment in GStonk style. ARG is not used for now."
  (interactive "*P")
  (require 'newcomment)
  (save-excursion
    (let ((comment-start ";;") (comment-end "")
          (comment-column 0)
          (comment-style 'plain))
      (unless (use-region-p)
        (end-of-line)
        (push-mark (line-beginning-position))
        (setq mark-active t))
      (comment-dwim nil))))


(defconst gstonk-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?\; ". 12b" st)
    (modify-syntax-entry ?\n "> b" st)
    (modify-syntax-entry ?\' "\"" st)
    (modify-syntax-entry ?# "w" st)
    (modify-syntax-entry ?` "w" st)
    (modify-syntax-entry ?. "_" st)
    (modify-syntax-entry ?- "." st)
    st)
  "Syntax table for Gstonk mode.")

;;TODO: remove this section of code if you don't have flycheck
(flycheck-define-checker gstonk
  "A GStonk syntax checker using the GStonk compiler."
  ;;TODO: add "gstonk" to path or provide the absolute path to the binary
  :command ("gstonk" "--flycheck" source)
  :error-patterns
  ((info line-start (file-name) ":" line ":" column ": message:" (message) line-end)
   (warning line-start (file-name) ":" line ":" column ": warning:" (message) line-end)
   (error line-start (file-name) ":" line ":" column ": error:" (message) line-end))
  :modes gstonk-mode
  )

(define-derived-mode gstonk-mode prog-mode "GStonk"
  :syntax-table gstonk-syntax-table
  (use-local-map gstonk-map)
  (flycheck-select-checker 'gstonk)
  (setq font-lock-defaults '(gstonk-font-lock))
  (setq indent-line-function 'gstonk-indent-line)
  (local-set-key [remap comment-dwim] 'gstonk-comment))


;;;###autoload
(add-to-list 'auto-mode-alist '("\\.gst\\'" . gstonk-mode))

(provide 'gstonk-mode)
;;; gstonk-mode.el ends here
