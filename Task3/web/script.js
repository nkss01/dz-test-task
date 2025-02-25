const URL = `http://localhost:3000/api/`
const SEARCH_LIMIT = 5;
const MIN_DATE = -8640000000000000;
const MAX_DATE = 8640000000000000;

const app = {
	init: function() {
		taskList.init();
		search.init();
		modal.init();
		sorter.init();
		dateRange.init();
		falseOnlyCheckbox.init();
	},
	
	send: function(method, callback) {
		fetch(`${URL}${method}`, {
			method: "GET",
			headers: {
				"Accept": "application/json"
			}
		})
		.then(response => {
			if (!response.ok) {
				alert("Ошибка соединения: " + response.statusText);
			}
			return response.json();
		})
		.then(data => {
			callback(data);
		})
		.catch(error => {
			alert("Ошибка запроса: ", error);
		});
	},
}

const taskList = {
	list: document.getElementById("taskList"),
	tasks: null,
	
	init: function() {
		app.send(`todos`, (data) => {
			taskList.tasks = data;
		});
	},
	update: function(newTasks) {
		const range = dateRange.getRange();
		const startDate = range.start;
		const endDate = range.end;
		const falseStatusOnly = falseOnlyCheckbox.isActive();
		newTasks = newTasks.filter(task => {
			const taskDate = new Date(task.date);
			return taskDate >= startDate && taskDate <= endDate
			       && (falseStatusOnly ? (task.status == false) : true);
		});
		
        newTasks.sort((a, b) => {
            return sorter.isAscending ? new Date(a.date) - new Date(b.date) : new Date(b.date) - new Date(a.date);
        });

        taskList.list.innerHTML = "";
        newTasks.forEach((task, index) => {
            const taskElement = document.createElement("div");
            taskElement.classList.add("task");

            taskElement.innerHTML = `
                <div>
                    <strong>${task.name}</strong>
                    <p>${task.shortDesc}</p>
                    <small>${new Date(task.date).toLocaleString()}</small>
                </div>
                <div class="checkboxContainer">
                    <input type="checkbox" id="checkbox-${index}" class="checkbox disabled" ${task.status ? "checked" : ""}>
                    <label for="checkbox-${index}" class="checkboxLabel ${task.status ? "checked" : ""}">✔</label>
                </div>
            `;

            setTimeout(() => {
                taskElement.classList.add("visible");
            }, 0);

            taskElement.addEventListener("click", function() {
                modal.show(task);
            });

            taskList.list.appendChild(taskElement);
        });
	}
}

const search = {
	input: document.getElementById('search'),
	list: document.getElementById('suggestions'),
	
	init: function() {
		this.input.addEventListener('input', () => {
			search.list.innerHTML = '';
			if(!search.input.value) {
				return;
			}
			app.send(`todos/find?q=${search.input.value}&limit=${SEARCH_LIMIT}`, (data) => {
				if(data) {
					search.showList(data);
				}
			});
		});
	},
	showList: function(tasks) {
		if (tasks) {
			tasks.forEach(task => {
				const suggestionItem = document.createElement('div');
				suggestionItem.classList.add('suggestion-item');
				suggestionItem.textContent = task.name;

				suggestionItem.addEventListener('click', () => {
					modal.show(task);
					search.list.innerHTML = '';
				});

				search.list.appendChild(suggestionItem);
			});
		}		
	}
}

const falseOnlyCheckbox = {
	input: document.getElementById('falseStatus'),
	
	init: function() {
		this.input.addEventListener('change', function() {
			taskList.update(taskList.tasks);
		});
	},
	
	isActive: function() {
		return this.input.checked;
	}
}

const modal = {
    win: document.getElementById('modal'),
    title: document.getElementById('modalTitle'),
    headerCheckbox: document.getElementById('modalCheckbox'),
    date: document.getElementById('modalDate'),
    desc: document.getElementById('modalDescription'),
    doneButton: document.getElementById('modalDoneButton'),
	
	init: function() {
		this.doneButton.addEventListener('click', this.hide);
	},
	
    show: function(task) {
        modal.title.textContent = task.name;
        modal.headerCheckbox.checked = task.status;
        modal.date.textContent = `${new Date(task.date).toLocaleString()}`;
        modal.desc.textContent = task.fullDesc;

        const checkboxLabel = modal.win.querySelector('.checkboxLabel');

        if (task.status) {
            modal.headerCheckbox.checked = true;
            checkboxLabel.classList.add('checked');
        } else {
            checkboxLabel.classList.remove('checked');
        }

        modal.win.classList.add('show');
        
        setTimeout(() => {
            modal.win.querySelector('.modalContent').style.opacity = 1;
        }, 0);
    },
	
    hide: function() {
        modal.win.classList.remove('show');
        setTimeout(() => {
            modal.win.querySelector('.modalContent').style.opacity = 0;
        }, 300);
    }
}

const sorter = {
	isAscending: true,
	
	init: function() {
		document.getElementById('sortButton').addEventListener('click', function() {
			sorter.isAscending = !sorter.isAscending;
			this.textContent = sorter.isAscending ? 'Сортировать по дате: По возрастанию' : 'Сортировать по дате: По убыванию';
			taskList.update(taskList.tasks);
		});
	}
}

const dateRange = {
	startDate: document.getElementById("startDate"),
	endDate: document.getElementById("endDate"),
	todayButton: document.getElementById("today"),
	weekButton: document.getElementById("week"),
	
	init: function() {
		this.startDate.addEventListener("input", this.onInput);
		this.endDate.addEventListener("input", this.onInput);
		this.todayButton.addEventListener("click", this.setToday);
		this.weekButton.addEventListener("click", this.setWeek);
		dateRange.onDateChange();
	},
	getRange: function() {
		return {
			start: new Date(startDate.value || MIN_DATE),
			end: new Date(endDate.value || MAX_DATE)
		};
	},
	onDateChange: function() {
		app.send(`todos`, (data) => {
			taskList.update(data);
		});
	},
	
	formatDate: function(date) {
		return date.toISOString().split("T")[0];
	},
	setToday: function() {
		const today = dateRange.formatDate(new Date());
		dateRange.startDate.value = today;
		dateRange.endDate.value = today;
		dateRange.onDateChange();
	},
	setWeek: function() {
		const today = new Date();
		const firstDay = new Date(today);
		const lastDay = new Date(today);
		firstDay.setDate(today.getDate() - today.getDay() + 1);
		lastDay.setDate(today.getDate() - today.getDay() + 7);
    
		dateRange.startDate.value = dateRange.formatDate(firstDay);
		dateRange.endDate.value = dateRange.formatDate(lastDay);
		dateRange.onDateChange();
	},
	onInput: function() {
		dateRange.onDateChange();
	},
}

document.addEventListener("DOMContentLoaded", function() {
	app.init();
});